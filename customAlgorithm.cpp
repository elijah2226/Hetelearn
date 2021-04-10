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

//	ȫ�ֱ���
map<string,map<string,map<pair<string,double>,set<string>>>> P;
vector<set<vector<string>>> allPath;
const double e= 2.7182818284, CONVERGE=pow(10,-3);

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


//	���ؽ���ĳ�����͵�����
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

//	����ת�Ƹ��ʾ��� 
void transitionMatrix(const HIN &hin, const map<string,double> &para, map<string,map<string,map<pair<string,double>,set<string>>>>& transMatrix){
	set<string> neibrNodeList;
	map<pair<string,double>, set<string>> neibrTypePro;
	map<string, map<pair<string,double>, set<string>>> neibrNodeTypePro;
	string node, nodeType;
//	��ͼ��nodelist��ȡ����㼯�ϼ������� 
	for(auto nodeTypes=hin.nodeList.begin(); nodeTypes!=hin.nodeList.end(); nodeTypes++){
		nodeType = nodeTypes->first;
		neibrNodeTypePro.clear();
//		��ȡ������͵ı����ͼ��� 
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
			
//			�ڱ߼����м���ĳ���ĸ���ߵ�������ͬʱ���Եõ�ת�Ƹ��� 
			for(auto iter2=nodeLinkTypeList.begin(); iter2!=nodeLinkTypeList.end(); iter2++){
				int nodeTypeCount;
				double posibility; 
//				��ȡ����ض��ߵ�����
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
void BRPtrain(const HIN &hin, const map<string,map<string,map<pair<string,double>,set<string>>>> &inP, map<string,double> &Para, map<string,set<string>> &posImpFB, string start, string end){
	P = inP;
//	��ȡk�����¸�������Ԫ·�� 
	getPath(&hin, k, start, end, &allPath);
//	��Ʒ��㼯��
	set<string> allItem, negItem, posItem;
	auto endNodeType = hin.nodeList.find(end);
	allItem.insert(endNodeType->second.begin(), endNodeType->second.end());
//  �û���㼯��
	auto nodeType = hin.nodeList.find(start);

//	ѵ����ģ
	int count=1;
	
	while(true){
//		���ѡȡ�û����
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
//		������������
		posItem = posImpFB[*istStart];
		if(posItem.size()==0){
//			û��������
			continue;
		}
		negItem.clear();
		set_difference(allItem.begin(), allItem.end(), posItem.begin(), posItem.end(), inserter(negItem, negItem.begin()));
//		���һ����������ֱ����negItem.begin()����Ϊ��set_differentʵ���У������ֱ�ӱ���ֵ�ģ�����set������Ԫ�ز���ֱ�ӱ���ֵ 

//		�������������Ʒ
		auto posI = select_randomly(posItem.begin(), posItem.end());
		auto negI = select_randomly(negItem.begin(), negItem.end());

		
		double posEtm = RWR(*istStart, *posI);
		double negEtm = RWR(*istStart, *negI);
		double estimateDif = posEtm - negEtm;
//		�Բ���ϵ�н��е��� 
		cout << count++ << endl;
		bool Convg=true;
		for(auto para=Para.begin(); para!=Para.end(); para++){
//			�ѱ�����ת����pair��ʽ��ͷβ���
			tuple<string, string, double> linkType;
			for(auto linksType=hin.linkTypeList.begin(); linksType!=hin.linkTypeList.end(); linksType++){
				if(get<2>(*linksType)==para->first){
					get<0>(linkType)=get<0>(*linksType);
					get<1>(linkType)=get<1>(*linksType);
					get<2>(linkType)=para->second;
					break;
				}
			}
			
//			n��ѧϰ���ʲ���
			double posTypeEtm = RWR(*istStart, *posI, &linkType);
			double negTypeEtm = RWR(*istStart, *negI, &linkType);
			double n=7;
			double grad = 1/(1+pow(e,estimateDif)) * (posTypeEtm-negTypeEtm);
//			���⸺Ȩֵ�Ĳ���
			while(para->second+n*grad<0){
				n *= 0.5;
			}
			para->second += n*grad;
			//������� 
			if(abs(grad)>CONVERGE){
				Convg=false;
			}
		}
		if(Convg) break;
//		����ת�Ƹ��ʾ���
		transitionMatrix(hin, Para, P);
	}
}

struct cmp
{
	bool operator ()(pair<string,double> a , pair<string,double> b){
		return a.second > b.second;
	}
};


// Ԥ��
string predict(const HIN &hin, const map<string,map<string,map<pair<string,double>,set<string>>>> &inP, vector<pair<string, double>> &topK, string user, string start, string end, int k){
	priority_queue<pair<string,double>, vector<pair<string,double>>, cmp> topKList;

//  ���������ʼ��
	if(!user.size()){
		auto nodeType = hin.nodeList.find(start);
		auto istStart = select_randomly(nodeType->second.begin(), nodeType->second.end());
		user = *istStart;
	}
//	cout << "user:" << user << endl;


//	����·�� 
	P = inP;
	getPath(&hin, k, start, end, &allPath);
	
	
//	Ŀ���㼯�� 
	auto endNodeList = hin.nodeList.find(end);
	set<string> items;
	items.insert(endNodeList->second.begin(), endNodeList->second.end());
	cmp cm;
	
	
//	����Ŀ���� 
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
	
	
//	������
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


// �Զ���
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
// ��׼����
double baseLine(const HIN &hin,const vector<pair<string, double>> &topK, string user, string start, string end){
//  �ҽڵ�����
	string linkType;
	for(auto linkTypes=hin.linkTypeList.begin(); linkTypes!=hin.linkTypeList.end(); linkTypes++){
		if((get<0>(*linkTypes)==start&&get<1>(*linkTypes)==end)||(get<1>(*linkTypes)==start&&get<0>(*linkTypes)==end)){
			linkType = get<2>(*linkTypes);
		}
	}
	
//  ����յ�
	auto linksIter = hin.linkList.find(linkType);
	auto links = linksIter->second;
	set<pair<string,int>> endNodes;
	int n = linkDetail(links, endNodes, user);
	
//	cout << "ʵ�����ӵĽڵ�:" << endl;
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
	
//	cout << "rank���飺" << endl;
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



