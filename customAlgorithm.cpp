#include<iostream>
#include"HIN.h"
#include<algorithm> 
#include<random>
#include<iterator>
#include<cmath>
#include<fstream>
#include<ctime>
#include<queue>
using namespace std;

//	全局变量
map<string,map<string,map<pair<string,double>,set<string>>>> P;
vector<set<vector<string>>> allPath;
const double e= 2.7182818284, CONVERGE=5*pow(10,-4);

//	随机数生成器
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g){
	uniform_int_distribution<> dis(0, distance(start, end)-1);
//	传入的mt19937生成的随机数给均匀分布生成器dis作为种子 
	advance(start, dis(g));
	return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
//  random_device生成随机数给mt19937作为种子 
    static mt19937 gen(time(0));
    return select_randomly(start, end, gen);
}

//	找到指定结点的指定边类型的数量 
int neibrNodeTypeCount(const map<string, set<tuple<string, string, double>>> *nodeLinkList, set<string> *neibrNodeList, string node, string neibrType){
	neibrNodeList->clear();
	int count = 0;
	for(auto iter=nodeLinkList->begin(); iter!=nodeLinkList->end(); iter++){
		if(iter->first==neibrType){
			for(auto iter2=(iter->second).begin(); iter2!=(iter->second).end(); iter2++){
				if(get<0>(*iter2)==node){
					count++;
					neibrNodeList->emplace(get<1>(*iter2)); 
				}else if(get<1>(*iter2)==node){
					count++;
					neibrNodeList->emplace(get<0>(*iter2));
				}
			}
			break;
		}
	}
	return count;
}

//	计算转移概率矩阵 
void transitionMatrix(const HIN *hin,const map<string,double> *para, map<string,map<string,map<pair<string,double>,set<string>>>> *transMatrix){
	set<string> neibrNodeList;
	map<pair<string,double>, set<string>> neibrTypePro;
	map<string, map<pair<string,double>, set<string>>> neibrNodeTypePro;
	string node, nodeType;
//	从图的nodelist中取出结点集合及其类型 
	for(auto iter=hin->nodeList.begin(); iter!=hin->nodeList.end(); iter++){
		nodeType = iter->first;
		neibrNodeTypePro.clear();
//		获取结点类型的边类型集合 
		map<string, string> nodeLinkTypeList;
		for(auto iter1=hin->linkTypeList.begin(); iter1!=hin->linkTypeList.end(); iter1++){
			if(get<0>(*iter1)==nodeType){
				nodeLinkTypeList.emplace(get<2>(*iter1), get<1>(*iter1));
			}else if(get<1>(*iter1)==nodeType){
				nodeLinkTypeList.emplace(get<2>(*iter1), get<0>(*iter1));
			}
		}
		
		for(auto iter1=iter->second.begin(); iter1!=iter->second.end(); iter1++){
			node = *iter1;
			neibrTypePro.clear();
			
//			在边集合中计算某结点的各类边的数量，同时可以得到转移概率 
			for(auto iter2=nodeLinkTypeList.begin(); iter2!=nodeLinkTypeList.end(); iter2++){
				int nodeTypeCount;
				double posibility; 
//				获取结点特定边的数量
				nodeTypeCount = neibrNodeTypeCount(&(hin->linkList), &neibrNodeList, node, iter2->first);
				if(nodeTypeCount){
					posibility = (para->find(iter2->first))->second*(1.0/nodeTypeCount);
					pair<string, double> neibrTypeP(iter2->second, posibility);
					neibrTypePro.emplace(neibrTypeP, neibrNodeList); 
				}
			}
			neibrNodeTypePro.emplace(node, neibrTypePro);
		}
		transMatrix->emplace(nodeType, neibrNodeTypePro);
	}
};

//	计算反馈矩阵，dir表示implicitFB的方向
void implicitFB(const set<tuple<string,string, double>> *linkList, map<string,set<string>> *posImpFB, int pos, int dir){
	string node;
	if(dir){
		for(auto link=linkList->begin(); link!=linkList->end(); link++){
			node = get<0>(*link);
			if(pos==-1){
				(*posImpFB)[node].emplace(get<1>(*link));
			}else if(get<2>(*link)==pos){
				(*posImpFB)[node].emplace(get<1>(*link));
			}
		}
	}else{
		for(auto link=linkList->begin(); link!=linkList->end(); link++){
			node = get<1>(*link);
			if(pos==-1){
//				无权值情况,有边即为正反馈 
				(*posImpFB)[node].emplace(get<0>(*link));
			}else if(get<2>(*link)==pos){
				(*posImpFB)[node].emplace(get<0>(*link));
			}
		}
	} 
}

//	递归获取路径的函数
void getAllPath(int length, int k, map<string, set<string>> nodeTypeGo, string now, set<vector<string>> *allpath, vector<string> istPath){
	length++;
	for(auto iter=nodeTypeGo[now].begin(); iter!=nodeTypeGo[now].end(); iter++){
		if(length < k){
			string cur = *iter;
			istPath.push_back(cur);
			getAllPath(length, k, nodeTypeGo, cur, allpath, istPath);
			istPath.pop_back();
		}else{
			string last = *iter;
			istPath.push_back(last);
			allpath->emplace(istPath);
			istPath.pop_back();
		}
	}
}

//	获取不同步长下的元路径 
void getPath(const HIN *hin, int k, string start, string end, vector<set<vector<string>>>* path){
//	获取结点类型的邻居结点类型 
	map<string, set<string>> nodeTypeGo;
	string nodeType;
	for(auto iter=(hin->nodeTypeList).begin(); iter!=(hin->nodeTypeList).end(); iter++){
		nodeType=*iter;
		set<string> temp;
		for(auto iter1=hin->linkTypeList.begin(); iter1!=hin->linkTypeList.end(); iter1++){
			if(get<0>(*iter1)==nodeType){
				temp.emplace(get<1>(*iter1));
			}else if(get<1>(*iter1)==nodeType){
				temp.emplace(get<0>(*iter1));
			}
		}
		nodeTypeGo.emplace(nodeType, temp);
	}
//	获取不同步长下的路径
	int i = 1;
	while(i<=k){
		set<vector<string>> allpath;
		vector<string> istPath;
		set<vector<string>> iPath;
		istPath.push_back(start);
		getAllPath(0, i, nodeTypeGo, start, &allpath, istPath);
//		筛选
		for(auto iter=allpath.begin(); iter!=allpath.end(); iter++){
			auto last = iter->end();
			last--;
			if(*(iter->begin())==start && *last==end){
				iPath.emplace(*iter); 
			}
		}
		path->push_back(iPath);
		i++;
	}
}

//实例结点的特定类型邻居集合，同时修改概率 
set<string> nodeGo(string now, string nowType, string nextType, double* posibility, double para=0){
	for(auto nodeType=P.begin(); nodeType!=P.end(); nodeType++){
//		结点类型 
		if(nodeType->first==nowType){
			for(auto node=nodeType->second.begin(); node!=nodeType->second.end(); node++){
//				结点 
				if(node->first==now){
					for(auto goType=node->second.begin(); goType!=node->second.end(); goType++){
//						下个结点类型 
						if(goType->first.first==nextType){
							if(para){
								*posibility *= goType->first.second/para;
							}else{
								*posibility *= goType->first.second;
							}
							return goType->second;
						}
					}
//					考虑没有目标结点类型的情况	
					set<string> empty;
					return empty;
				}
			}
		}
	}
}

void typePathP(string now, string des,set<string> next, double posibility, int curStep, int step, vector<string> path, double* allPos, int index=-1, double para=0){
	curStep++;
	for(auto nextNode=next.begin(); nextNode!=next.end(); nextNode++){
		if(curStep<step){
			double tempPosibility=posibility;
			set<string> tempNextNode;
			if(curStep==index){
				tempNextNode = nodeGo(*nextNode, path[curStep], path[curStep+1], &tempPosibility, para);
			}else{
				tempNextNode = nodeGo(*nextNode, path[curStep], path[curStep+1], &tempPosibility);
			}
//			检查是否有特定类型的下结点，避免越界 
			if(tempNextNode.size()){
				typePathP(*nextNode, des, tempNextNode, tempPosibility, curStep, step, path, allPos, para);
			}
		}else if(*nextNode==des){
//			存在目标结点
			*allPos += posibility;
			break;
		}
	}
}

//	step长度下的P概率 
double countP(string user, string item, int step, tuple<string, string, double> *linkType=NULL){
	double countOneP=0;
//	获取长度为step(k)的元路径
	set<vector<string>> kPath=allPath[step-1];
	if(linkType==NULL){
//		计算似然概率 
//		遍历各种类型的元路径 
		for(auto path=kPath.begin(); path!=kPath.end(); path++){
			double allPos=0;		
			int curStep=0;
			double posibility=1;
			auto firstGo=nodeGo(user, (*path)[curStep], (*path)[curStep+1], &posibility);
			typePathP(user, item, firstGo, posibility, curStep, step, *path, &allPos);
			countOneP+=allPos; 
		}
	}else{
//		计算梯度 
//		对元路径进行标记
		map<int, set<vector<string>>> markPath;
		for(auto path=kPath.begin(); path!=kPath.end(); path++){
			int index=0;
			for(auto node=path->begin(); node!=path->end()-1; node++, index++){
				if((*node==get<0>(*linkType)&&*(node+1)==get<1>(*linkType))||(*node==get<1>(*linkType)&&*(node+1)==get<0>(*linkType))){
					auto check = markPath.find(index);
					if(check!=markPath.end()){
	//					存在
						check->second.emplace(*path);
					}else{
					 	set<vector<string>> temp;
					 	temp.emplace(*path);
						markPath.emplace(index, temp); 
					}
				}
			}
		}		
		for(auto mark=markPath.begin(); mark!=markPath.end(); mark++){
			for(auto path=mark->second.begin(); path!=mark->second.end(); path++){
				double allPos=0;
				int curStep=0;
				double posibility=1;
				set<string> firstGo; 
				if(mark->first==0){
					firstGo=nodeGo(user, (*path)[curStep], (*path)[curStep+1], &posibility, get<2>(*linkType));
				}else{
					firstGo=nodeGo(user, (*path)[curStep], (*path)[curStep+1], &posibility);
				}
				typePathP(user, item, firstGo, posibility, curStep, step, *path, &allPos, mark->first, get<2>(*linkType));
				countOneP+=allPos;
			}
		}
	}
	return countOneP;
}

double a=0.8;
int k=3;
//	RWR模型 
double RWR(string user, string item, tuple<string, string, double> *linkType=NULL){
	double result=0.0;
	int step=k; 
	while(step){ 
		if(step==k){
			result += pow(a, step)*countP(user, item, step, linkType);
		}else{
			result += pow(a, step)*(1-a)*countP(user, item, step, linkType);
		}
		step--;
	}
	return result;
}

//	基于BPR模型下的边权值的机器学习 
void BRPtrain(const HIN *hin, const map<string,map<string,map<pair<string,double>,set<string>>>> *inP, map<string,double> *Para, map<string,set<string>> *posImpFB, string start, string end){
	P = *inP;
//	参数迭代
	auto nPara=*Para, lPara=*Para; 
//	获取k及以下各步长的元路径 
	getPath(hin, k, start, end, &allPath);
//	目标结点集合
	set<string> allItem, negItem, posItem;
	auto endNodeType = hin->nodeList.find(end);
	allItem.insert(endNodeType->second.begin(), endNodeType->second.end());
	
	auto nodeType = hin->nodeList.find(start);
//	cout <<  nodeType->first << endl;

//	训练规模
	int count=1, notConvg=0;
	double estimateDif, posEtm;
	while(true){
		cout << count << endl;
//		随机选取结点迭代拟合（一个点就是迭代一次）
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
//		正负反馈集合
		posItem=(*posImpFB)[*istStart];
		negItem.clear();
		set_difference(allItem.begin(), allItem.end(), posItem.begin(), posItem.end(), inserter(negItem, negItem.begin()));
//		最后一个参数不能直接用negItem.begin()，因为在set_different实现中，结果是直接被赋值的，但是set容器的元素不能直接被赋值 
		if(posItem.size()==0){
//		没有正反馈
			continue; 
		}
//		随机正负反馈物品
		auto posI = select_randomly(posItem.begin(), posItem.end());
		posEtm = RWR(*istStart, *posI);
		auto negI = select_randomly(negItem.begin(), negItem.end());
		estimateDif = posEtm-RWR(*istStart, *negI);
//		对参数系列进行迭代 
		for(auto para=nPara.begin(); para!=nPara.end(); para++){
//		把边类型转化成pair形式的头尾结点
			tuple<string, string, double> linkType;
			for(auto linksType=hin->linkTypeList.begin(); linksType!=hin->linkTypeList.end(); linksType++){
				if(get<2>(*linksType)==para->first){
					get<0>(linkType)=get<0>(*linksType);
					get<1>(linkType)=get<1>(*linksType);
					get<2>(linkType)=para->second;
					break;
				}
			}
			double n=7;
			double grad = 1/(1+pow(e, estimateDif))*(RWR(*istStart, *posI, &linkType)-RWR(*istStart, *negI, &linkType));
			while(para->second+n*grad<0){
				n *= 0.5;
			}
			para->second += n*grad;
			//收敛检测 
			if(abs(grad)>CONVERGE){
				notConvg=1;
			}
		}
		lPara = nPara;
//		更新转移概率矩阵
		transitionMatrix(hin, &lPara, &P);
		count++;
		if(notConvg==0){
//			更新传入的参数列表
			cout << count << endl;
			*Para=lPara;
			break;
		}
	}
}


bool lessPair(pair<string,int> a, pair<string,int> b){
	return get<1>(a) < get<1>(b);
}


void predict(const HIN &hin, const map<string,map<string,map<pair<string,double>,set<string>>>> &inP, vector<pair<string, double>> &topK, string user, string start, string end, int k){
	priority_queue<pair<string,int>, vector<pair<string,int>>, decltype(&lessPair)> topKList;//(&lessPair)
//	生成路径 
	P = inP;
	getPath(&hin, k, start, end, &allPath);
	
//	目标结点集合 
	auto endNodeList = hin.nodeList.find(end);
	set<string> items;
	items.insert(endNodeList->second.begin(), endNodeList->second.end());
	
//	遍历目标结点 
	for(auto item=items.begin(); item!=items.end(); item++){
		pair<string, double> itemValue(*item, RWR(user, *item));
		
		if(topKList.size() < k){
			topKList.emplace(itemValue);
		}else if(lessPair(topKList.top(), itemValue)){
			topKList.pop();
			topKList.emplace(itemValue);
		}
	}
	
//	输出检查
	while(topKList.size()){
		cout << get<0>(topKList.top()) << ":" << get<1>(topKList.top()) << endl; 
	}
}




