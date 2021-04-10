#include<iostream>
#include"HIN.h"
#include<algorithm> 
#include<random>
#include<iterator>
#include<cmath>
#include<fstream>
#include<ctime>
#include<queue>
#include<typeinfo>
using namespace std;

//	全局变量
map<string,map<string,map<pair<string,double>,set<string>>>> P;
vector<set<vector<string>>> allPath;
const double e= 2.7182818284, CONVERGE=pow(10,-3);

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


//	返回结点的某边类型的数量
int neibrNodeTypeCount(const map<string, set<tuple<string, string, double>>> &nodeLinkList, set<string> *neibrNodeList, string node, string neibrType){
	neibrNodeList->clear();
	int count = 0;
	auto links = nodeLinkList.find(neibrType);
	for(auto link=(links->second).begin(); link!=(links->second).end(); link++){
		if(get<0>(*link)==node){
			count++;
			neibrNodeList->emplace(get<1>(*link));
		}else if(get<1>(*link)==node){
			count++;
			neibrNodeList->emplace(get<0>(*link));
		}
	}
	return count;
}

//	计算转移概率矩阵 
void transitionMatrix(const HIN &hin, const map<string,double> &para, map<string,map<string,map<pair<string,double>,set<string>>>>& transMatrix){
	set<string> neibrNodeList;
	map<pair<string,double>, set<string>> neibrTypePro;
	map<string, map<pair<string,double>, set<string>>> neibrNodeTypePro;
	string node, nodeType;
//	从图的nodelist中取出结点集合及其类型 
	for(auto nodeTypes=hin.nodeList.begin(); nodeTypes!=hin.nodeList.end(); nodeTypes++){
		nodeType = nodeTypes->first;
		neibrNodeTypePro.clear();
//		获取结点类型的边类型集合 
		map<string, string> nodeLinkTypeList;
		for(auto linkType=hin.linkTypeList.begin(); linkType!=hin.linkTypeList.end(); linkType++){
			if(get<0>(*linkType)==nodeType){
				nodeLinkTypeList.emplace(get<2>(*linkType), get<1>(*linkType));
			}else if(get<1>(*linkType)==nodeType){
				nodeLinkTypeList.emplace(get<2>(*linkType), get<0>(*linkType));
			}
		}
		for(auto iter1=nodeTypes->second.begin(); iter1!=nodeTypes->second.end(); iter1++){
			node = *iter1;
			neibrTypePro.clear();
			
//			在边集合中计算某结点的各类边的数量，同时可以得到转移概率 
			for(auto iter2=nodeLinkTypeList.begin(); iter2!=nodeLinkTypeList.end(); iter2++){
				int nodeTypeCount;
				double posibility; 
//				获取结点特定边的数量
				nodeTypeCount = neibrNodeTypeCount(hin.linkList, &neibrNodeList, node, iter2->first);
				if(nodeTypeCount){
					posibility = (para.find(iter2->first))->second*(1.0/nodeTypeCount);
					pair<string, double> neibrTypeP(iter2->second, posibility);
					neibrTypePro.emplace(neibrTypeP, neibrNodeList); 
				}
			}
			neibrNodeTypePro.emplace(node, neibrTypePro);
		}
		transMatrix.emplace(nodeType, neibrNodeTypePro);
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
void BRPtrain(const HIN &hin, const map<string,map<string,map<pair<string,double>,set<string>>>> &inP, map<string,double> &Para, map<string,set<string>> &posImpFB, string start, string end){
	P = inP;
//	获取k及以下各步长的元路径 
	getPath(&hin, k, start, end, &allPath);
//	物品结点集合
	set<string> allItem, negItem, posItem;
	auto endNodeType = hin.nodeList.find(end);
	allItem.insert(endNodeType->second.begin(), endNodeType->second.end());
//  用户结点集合
	auto nodeType = hin.nodeList.find(start);

//	训练规模
	int count=1;
	
	while(true){
//		随机选取用户结点
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
//		正负反馈集合
		posItem = posImpFB[*istStart];
		if(posItem.size()==0){
//			没有正反馈
			continue;
		}
		negItem.clear();
		set_difference(allItem.begin(), allItem.end(), posItem.begin(), posItem.end(), inserter(negItem, negItem.begin()));
//		最后一个参数不能直接用negItem.begin()，因为在set_different实现中，结果是直接被赋值的，但是set容器的元素不能直接被赋值 

//		随机正负反馈物品
		auto posI = select_randomly(posItem.begin(), posItem.end());
		auto negI = select_randomly(negItem.begin(), negItem.end());

		
		double posEtm = RWR(*istStart, *posI);
		double negEtm = RWR(*istStart, *negI);
		double estimateDif = posEtm - negEtm;
//		对参数系列进行迭代 
		cout << count++ << endl;
		bool Convg=true;
		for(auto para=Para.begin(); para!=Para.end(); para++){
//			把边类型转化成pair形式的头尾结点
			tuple<string, string, double> linkType;
			for(auto linksType=hin.linkTypeList.begin(); linksType!=hin.linkTypeList.end(); linksType++){
				if(get<2>(*linksType)==para->first){
					get<0>(linkType)=get<0>(*linksType);
					get<1>(linkType)=get<1>(*linksType);
					get<2>(linkType)=para->second;
					break;
				}
			}
			
//			n：学习速率参数
			double posTypeEtm = RWR(*istStart, *posI, &linkType);
			double negTypeEtm = RWR(*istStart, *negI, &linkType);
			double n=7;
			double grad = 1/(1+pow(e,estimateDif)) * (posTypeEtm-negTypeEtm);
//			避免负权值的产生
			while(para->second+n*grad<0){
				n *= 0.5;
			}
			para->second += n*grad;
			//收敛检测 
			if(abs(grad)>CONVERGE){
				Convg=false;
			}
		}
		if(Convg) break;
//		更新转移概率矩阵
		transitionMatrix(hin, Para, P);
	}
}

struct cmp
{
	bool operator ()(pair<string,double> a , pair<string,double> b){
		return a.second > b.second;
	}
};


// 预测
string predict(const HIN &hin, const map<string,map<string,map<pair<string,double>,set<string>>>> &inP, vector<pair<string, double>> &topK, string user, string start, string end, int k){
	priority_queue<pair<string,double>, vector<pair<string,double>>, cmp> topKList;

//  随机生成起始点
	if(!user.size()){
		auto nodeType = hin.nodeList.find(start);
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
		user = *istStart;
	}
//	cout << "user:" << user << endl;


//	生成路径 
	P = inP;
	getPath(&hin, k, start, end, &allPath);
	
	
//	目标结点集合 
	auto endNodeList = hin.nodeList.find(end);
	set<string> items;
	items.insert(endNodeList->second.begin(), endNodeList->second.end());
	cmp cm;
	
	
//	遍历目标结点 
	for(auto item=items.begin(); item!=items.end(); item++){
		double value = RWR(user, *item);
		pair<string, double> itemValue(*item, value);
		
		if(topKList.size() < k){
			topKList.emplace(itemValue);
		}else if(cm(itemValue, topKList.top())){
			topKList.pop();
			topKList.emplace(itemValue);
		}
	}
	
	
//	输出检查
	reverse(topK.begin(), topK.end());
//	cout << "topK:" << endl;
	while(topKList.size()){
		topK.emplace_back(make_pair(topKList.top().first, topKList.top().second));
//		cout << topKList.top().first << ":" << topKList.top().second << endl;
		topKList.pop();
	}
	return user;
}


struct posLink{
	string item;
	int rate;
	bool operator ()(pair<string,int> link){
		if(link.second==rate && link.first==item) return true;
		else return false;
	}
};


// 自定义
int linkDetail(const set<tuple<string,string,double>> &links, set<pair<string,int>> &endNodeList, string node){
	endNodeList.clear();
	int count = 0;
	for(auto link=links.begin(); link!=links.end(); link++){
		if(get<0>(*link)==node){
			count++;
			endNodeList.emplace(get<1>(*link),get<2>(*link));
		}else if(get<1>(*link)==node){
			count++;
			endNodeList.emplace(get<0>(*link),get<2>(*link));
		}
	}
	return count;
}

double posRank(vector<int>& rank, int pos){
	int value = rank[pos];
	int bef=0, aft=0, index=0;
	while(index<pos){
		if(rank[index++]<value) bef++;
	}
	while(index<rank.size()){
		if(rank[index++]<value) aft++;
	}
	
	double res;
	if(aft&&bef) res = 1.0*aft/(bef+aft);
	else if(bef) res = 1.0/bef;
	else res = 1.0;
	
	return res;
}
// 基准测试
double baseLine(const HIN &hin,const vector<pair<string, double>> &topK, string user, string start, string end){
//  找节点类型
	string linkType;
	for(auto linkTypes=hin.linkTypeList.begin(); linkTypes!=hin.linkTypeList.end(); linkTypes++){
		if((get<0>(*linkTypes)==start&&get<1>(*linkTypes)==end)||(get<1>(*linkTypes)==start&&get<0>(*linkTypes)==end)){
			linkType = get<2>(*linkTypes);
		}
	}
	
//  求出终点
	auto linksIter = hin.linkList.find(linkType);
	auto links = linksIter->second;
	set<pair<string,int>> endNodes;
	int n = linkDetail(links, endNodes, user);
	
//	cout << "实际连接的节点:" << endl;
//	for(auto iter=endNodes.begin(); iter!=endNodes.end(); iter++){
//		cout << iter->first << ":" << iter->second << endl;
//	}
	
    vector<int> rank;
    int flag;
	posLink posL;
	
	for(auto item=topK.begin(); item!=topK.end(); item++){
		posL.item = item->first;
		flag = 0;
		for(int rate=0; rate<3; rate++){
			posL.rate = rate;
			auto iter = find_if(endNodes.begin(), endNodes.end(), posL);
			if(iter != endNodes.end()){
				rank.emplace_back(rate);
				flag = 1;
				break;
			}
		}
		if(!flag){
			rank.emplace_back(-1);
		}
	}
	
//	cout << "rank数组：" << endl;
//	for(auto it:rank){
//		cout << it << " ";
//	}
//	cout << endl;
	
	
	int nub = endNodes.size();
	int r = rank.size()-count(rank.begin(), rank.end(), -1);

	double base1=1;
	if(nub){
		if(nub<topK.size()){
			base1 = 1.0*r/nub;
		}else{
			base1 = 1.0*r/rank.size();
		}
	}

	double base2=0;
	int pos = 0;
	for(auto nub1:rank){
		base2 += posRank(rank, pos++);
	}

	double res = 1.0*base1*base2/rank.size();
	return res;
}



