#include "csv2HIN.h" 
#include<random>
#include<ctime>

//	�������� 
extern void transitionMatrix(const HIN*, const map<string,double>*, map<string, map<string, map<pair<string,double>, set<string>>>>*);
extern void implicitFB(const set<tuple<string, string, double>>*, map<string, set<string>>*, int pos=2, int dir=1);
extern void BRPtrain(const HIN*, const map<string,map<string,map<pair<string,double>,set<string>>>>*, map<string,double>*, map<string, set<string>>*, string start="userID", string end="placeID");

int main(int argc, char** argv) {
//	�����ĵ���Ϣ������ת�������� 
	vector<tuple<string, int, int, int>> readFileOption = {make_tuple("data\\rating_final.csv", 0, 1, 2),
														   make_tuple("data\\userpayment.csv", 0, 1, -1),
														   make_tuple("data\\usercuisine.csv", 0, 1, -1),
														   make_tuple("data\\chefmozaccepts.csv", 0, 1, -1),
														   make_tuple("data\\chefmozcuisine.csv", 0, 1, -1),
														   make_tuple("data\\chefmozparking.csv", 0, 1, -1)
														  };
	HIN hin;
	for(auto file=readFileOption.begin(); file!=readFileOption.end(); file++){
		csv2HIN temp(&hin, get<0>(*file), get<1>(*file), get<2>(*file), get<3>(*file));	
	}
//	hin.printNetworkSchema();
//	hin.printHIN();

//	�����ʼ������ϵ�У��ȣ������͵ĸ��ʼ��ϣ�
	default_random_engine e(time(0));
	uniform_real_distribution<double> rd(0.3,0.7);
	map<string, double> para;
	for(auto iter=hin.linkTypeList.begin(); iter!=hin.linkTypeList.end(); iter++){
		para.emplace(get<2>(*iter), rd(e));
	}
	ofstream outPut;
	outPut.open("out.csv"); 
	
	outPut << "origin Para:" << endl;
	for(auto p=para.begin(); p!=para.end(); p++){
		outPut << p->first << ":" << p->second << endl;
	}

//	����ת�Ƹ��ʾ���
	map<string, map<string, map<pair<string,double>, set<string>>>> transMatrix;
	transitionMatrix(&hin, &para, &transMatrix);

//	��ʽ���� 
	auto rating = (hin.linkList)["rating"];
	map<string, set<string>> posImpFB;
	implicitFB(&rating, &posImpFB);

//	ѵ�� 
	BRPtrain(&hin, &transMatrix, &para, &posImpFB);
//	���ѧϰ��Ĳ����б�
	outPut << "after Para:" << endl;
	for(auto p=para.begin(); p!=para.end(); p++){
		outPut << p->first << ":" << p->second << endl;
	}
//	Ԥ�����
	 
	return 0;
}
