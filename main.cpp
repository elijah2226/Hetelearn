#include "csv2HIN.h" 
#include<random>
#include<ctime>

//	函数声明 
extern void transitionMatrix(const HIN*, const map<string,double>*, map<string, map<string, map<pair<string,double>, set<string>>>>*);
extern void implicitFB(const set<tuple<string, string, double>>*, map<string, set<string>>*, int pos=2, int dir=1);
extern void BRPtrain(const HIN*, const map<string,map<string,map<pair<string,double>,set<string>>>>*, map<string,double>*, map<string, set<string>>*, string start="userID", string end="placeID");

int main(int argc, char** argv) {
//	定义文档信息，方便转换成网络 
	vector<tuple<string, int, int, int>> readFileOption = {make_tuple("data\\rating_final.csv", 0, 1, 2),
														   make_tuple("data\\userpayment.csv", 0, 1, -1),
														   make_tuple("data\\usercuisine.csv", 0, 1, -1),
														   make_tuple("data\\chefmozaccepts.csv", 0, 1, -1),
														   make_tuple("data\\chefmozcuisine.csv", 0, 1, -1),
														   make_tuple("data\\chefmozparking.csv", 0, 1, -1)
														  };
	HIN hin;
	for(auto iter=readFileOption.begin(); iter!=readFileOption.end(); iter++){
		csv2HIN temp(&hin, get<0>(*iter), get<1>(*iter), get<2>(*iter), get<3>(*iter));	
	}
//	hin.printNetworkSchema();
//	hin.printHIN();

//	随机初始化参数系列（θ：边类型的概率集合）
	default_random_engine e(time(0));
	uniform_real_distribution<double> rd(0.3,0.7);
	map<string, double> para;
	for(auto iter=hin.linkTypeList.begin(); iter!=hin.linkTypeList.end(); iter++){
		para.emplace(get<2>(*iter), rd(e));
	}

//	生成转移概率矩阵
	map<string, map<string, map<pair<string,double>, set<string>>>> transMatrix;
	transitionMatrix(&hin, &para, &transMatrix);

//	隐式反馈 
	auto rating = (hin.linkList)["rating"];
	map<string, set<string>> posImpFB;
	implicitFB(&rating, &posImpFB);

//	训练 
	BRPtrain(&hin, &transMatrix, &para, &posImpFB);
//	预测输出
	 
	return 0;
}
