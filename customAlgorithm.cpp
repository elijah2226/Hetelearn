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

//	ȫ�ֱ���
map<string,map<string,map<pair<string,double>,set<string>>>> P;
vector<set<vector<string>>> allPath;
const double e= 2.7182818284, CONVERGE=5*pow(10,-4);

//	�����������
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g){
	uniform_int_distribution<> dis(0, distance(start, end)-1);
//	�����mt19937���ɵ�����������ȷֲ�������dis��Ϊ���� 
	advance(start, dis(g));
	return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
//  random_device�����������mt19937��Ϊ���� 
    static mt19937 gen(time(0));
    return select_randomly(start, end, gen);
}

//	�ҵ�ָ������ָ�������͵����� 
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

//	����ת�Ƹ��ʾ��� 
void transitionMatrix(const HIN *hin,const map<string,double> *para, map<string,map<string,map<pair<string,double>,set<string>>>> *transMatrix){
	set<string> neibrNodeList;
	map<pair<string,double>, set<string>> neibrTypePro;
	map<string, map<pair<string,double>, set<string>>> neibrNodeTypePro;
	string node, nodeType;
//	��ͼ��nodelist��ȡ����㼯�ϼ������� 
	for(auto iter=hin->nodeList.begin(); iter!=hin->nodeList.end(); iter++){
		nodeType = iter->first;
		neibrNodeTypePro.clear();
//		��ȡ������͵ı����ͼ��� 
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
			
//			�ڱ߼����м���ĳ���ĸ���ߵ�������ͬʱ���Եõ�ת�Ƹ��� 
			for(auto iter2=nodeLinkTypeList.begin(); iter2!=nodeLinkTypeList.end(); iter2++){
				int nodeTypeCount;
				double posibility; 
//				��ȡ����ض��ߵ�����
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

//	���㷴������dir��ʾimplicitFB�ķ���
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
//				��Ȩֵ���,�б߼�Ϊ������ 
				(*posImpFB)[node].emplace(get<0>(*link));
			}else if(get<2>(*link)==pos){
				(*posImpFB)[node].emplace(get<0>(*link));
			}
		}
	} 
}

//	�ݹ��ȡ·���ĺ���
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

//	��ȡ��ͬ�����µ�Ԫ·�� 
void getPath(const HIN *hin, int k, string start, string end, vector<set<vector<string>>>* path){
//	��ȡ������͵��ھӽ������ 
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
//	��ȡ��ͬ�����µ�·��
	int i = 1;
	while(i<=k){
		set<vector<string>> allpath;
		vector<string> istPath;
		set<vector<string>> iPath;
		istPath.push_back(start);
		getAllPath(0, i, nodeTypeGo, start, &allpath, istPath);
//		ɸѡ
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

//ʵ�������ض������ھӼ��ϣ�ͬʱ�޸ĸ��� 
set<string> nodeGo(string now, string nowType, string nextType, double* posibility, double para=0){
	for(auto nodeType=P.begin(); nodeType!=P.end(); nodeType++){
//		������� 
		if(nodeType->first==nowType){
			for(auto node=nodeType->second.begin(); node!=nodeType->second.end(); node++){
//				��� 
				if(node->first==now){
					for(auto goType=node->second.begin(); goType!=node->second.end(); goType++){
//						�¸�������� 
						if(goType->first.first==nextType){
							if(para){
								*posibility *= goType->first.second/para;
							}else{
								*posibility *= goType->first.second;
							}
							return goType->second;
						}
					}
//					����û��Ŀ�������͵����	
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
//			����Ƿ����ض����͵��½�㣬����Խ�� 
			if(tempNextNode.size()){
				typePathP(*nextNode, des, tempNextNode, tempPosibility, curStep, step, path, allPos, para);
			}
		}else if(*nextNode==des){
//			����Ŀ����
			*allPos += posibility;
			break;
		}
	}
}

//	step�����µ�P���� 
double countP(string user, string item, int step, tuple<string, string, double> *linkType=NULL){
	double countOneP=0;
//	��ȡ����Ϊstep(k)��Ԫ·��
	set<vector<string>> kPath=allPath[step-1];
	if(linkType==NULL){
//		������Ȼ���� 
//		�����������͵�Ԫ·�� 
		for(auto path=kPath.begin(); path!=kPath.end(); path++){
			double allPos=0;		
			int curStep=0;
			double posibility=1;
			auto firstGo=nodeGo(user, (*path)[curStep], (*path)[curStep+1], &posibility);
			typePathP(user, item, firstGo, posibility, curStep, step, *path, &allPos);
			countOneP+=allPos; 
		}
	}else{
//		�����ݶ� 
//		��Ԫ·�����б��
		map<int, set<vector<string>>> markPath;
		for(auto path=kPath.begin(); path!=kPath.end(); path++){
			int index=0;
			for(auto node=path->begin(); node!=path->end()-1; node++, index++){
				if((*node==get<0>(*linkType)&&*(node+1)==get<1>(*linkType))||(*node==get<1>(*linkType)&&*(node+1)==get<0>(*linkType))){
					auto check = markPath.find(index);
					if(check!=markPath.end()){
	//					����
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
//	RWRģ�� 
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

//	����BPRģ���µı�Ȩֵ�Ļ���ѧϰ 
void BRPtrain(const HIN *hin, const map<string,map<string,map<pair<string,double>,set<string>>>> *inP, map<string,double> *Para, map<string,set<string>> *posImpFB, string start, string end){
	P = *inP;
//	��������
	auto nPara=*Para, lPara=*Para; 
//	��ȡk�����¸�������Ԫ·�� 
	getPath(hin, k, start, end, &allPath);
//	Ŀ���㼯��
	set<string> allItem, negItem, posItem;
	auto endNodeType = hin->nodeList.find(end);
	allItem.insert(endNodeType->second.begin(), endNodeType->second.end());
	
	auto nodeType = hin->nodeList.find(start);
//	cout <<  nodeType->first << endl;

//	ѵ����ģ
	int count=1, notConvg=0;
	double estimateDif, posEtm;
	while(true){
		cout << count << endl;
//		���ѡȡ��������ϣ�һ������ǵ���һ�Σ�
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
//		������������
		posItem=(*posImpFB)[*istStart];
		negItem.clear();
		set_difference(allItem.begin(), allItem.end(), posItem.begin(), posItem.end(), inserter(negItem, negItem.begin()));
//		���һ����������ֱ����negItem.begin()����Ϊ��set_differentʵ���У������ֱ�ӱ���ֵ�ģ�����set������Ԫ�ز���ֱ�ӱ���ֵ 
		if(posItem.size()==0){
//		û��������
			continue; 
		}
//		�������������Ʒ
		auto posI = select_randomly(posItem.begin(), posItem.end());
		posEtm = RWR(*istStart, *posI);
		auto negI = select_randomly(negItem.begin(), negItem.end());
		estimateDif = posEtm-RWR(*istStart, *negI);
//		�Բ���ϵ�н��е��� 
		for(auto para=nPara.begin(); para!=nPara.end(); para++){
//		�ѱ�����ת����pair��ʽ��ͷβ���
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
			//������� 
			if(abs(grad)>CONVERGE){
				notConvg=1;
			}
		}
		lPara = nPara;
//		����ת�Ƹ��ʾ���
		transitionMatrix(hin, &lPara, &P);
		count++;
		if(notConvg==0){
//			���´���Ĳ����б�
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
//	����·�� 
	P = inP;
	getPath(&hin, k, start, end, &allPath);
	
//	Ŀ���㼯�� 
	auto endNodeList = hin.nodeList.find(end);
	set<string> items;
	items.insert(endNodeList->second.begin(), endNodeList->second.end());
	
//	����Ŀ���� 
	for(auto item=items.begin(); item!=items.end(); item++){
		pair<string, double> itemValue(*item, RWR(user, *item));
		
		if(topKList.size() < k){
			topKList.emplace(itemValue);
		}else if(lessPair(topKList.top(), itemValue)){
			topKList.pop();
			topKList.emplace(itemValue);
		}
	}
	
//	������
	while(topKList.size()){
		cout << get<0>(topKList.top()) << ":" << get<1>(topKList.top()) << endl; 
	}
}




