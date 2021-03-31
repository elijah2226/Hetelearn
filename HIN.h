#ifndef HIN_H
#define HIN_H
#include<vector>
#include<tuple>
#include<set>
#include<map>
#include<iostream>
using namespace std;

class HIN
{
	public:
//		data
//		network schema
		set<string> nodeTypeList;
		vector<tuple<string, string, string>> linkTypeList;
//		HIN
		map<string, set<string>> nodeList;
		map<string, set<tuple<string, string, double>>> linkList;


//		function
//		HIN init
		void initNodeList(string nodeType){
			set<string> temp;
			nodeList.emplace(nodeType, temp);
		}
		void initLinkList(string linkType){
			set<tuple<string, string, double>> temp;
			linkList.emplace(linkType, temp);
		}
//		network schema
		void addNodeType(string nodeType){
			nodeTypeList.emplace(nodeType); 
			initNodeList(nodeType);
		};
		void addLinkType(string startNodeType, string endNodeType, string linkType){
			tuple<string, string, string> tempTuple = make_tuple(startNodeType, endNodeType, linkType);
			linkTypeList.push_back(tempTuple);
			initLinkList(linkType);
		};
		void printNetworkSchema(){
			cout << "NetworkSchema:" << endl;
			cout << "NodeTypeList:" << endl;
			int i = 1;
			for(set<string>::iterator iter=nodeTypeList.begin(); iter!=nodeTypeList.end(); iter++, i++){
				cout << i << ":" << *iter << endl;
			}
			cout << "LinkTypeList:" << endl;
			i = 1;
			for(vector<tuple<string, string, string>>::iterator iter=linkTypeList.begin(); iter!=linkTypeList.end(); iter++, i++){
				cout << i << ":" << get<0>(*iter) << "--" << get<2>(*iter) << "-->" << get<1>(*iter) <<endl;
			}
		};
//		HIN
		void addNode(string nodeType, string node){
			nodeList[nodeType].emplace(node);
		}
		void addLink(string linkType, string startNode, string endNode, double linkValue){
			tuple<string, string, double> temp(startNode, endNode, linkValue);
			linkList[linkType].emplace(temp);
		}
		void printHIN(){
			cout << "HIN:" << endl;
			cout << "NodeList:" << endl;
			int i;
			for(map<string, set<string>>::iterator iter=nodeList.begin(); iter!=nodeList.end(); iter++){
				i = 1;
				cout << iter->first << endl;
				for(set<string>::iterator iter1=iter->second.begin(); iter1!=iter->second.end(); iter1++, i++){
					cout << i << ":" << *iter1 << endl;
				}
			}
			cout << "LinkList:" << endl;
			for(map<string, set<tuple<string, string, double>>>::iterator iter=linkList.begin(); iter!=linkList.end(); iter++){
				i = 1;
				cout << iter->first << endl;
				for(set<tuple<string, string, double>>::iterator iter1=iter->second.begin(); iter1!=iter->second.end(); iter1++, i++){
					cout << i << ":" << get<0>(*iter1) << "--" << get<2>(*iter1) << "-->" << get<1>(*iter1) << endl; 
				}
			}
		}
};

#endif
