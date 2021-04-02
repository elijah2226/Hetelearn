#ifndef CSV2HIN_H
#define CSV2HIN_H
#include "HIN.h"
#include<fstream>
#include<sstream>

//	TODO:3）考虑文件读取不到的问题 
class csv2HIN
{
	public:
//		data
		vector<string> title;
		
//		function
		int maxIndex(int a, int b, int c){
			return c>(a>b?a:b)?c:(a>b?a:b);
		}
		
		csv2HIN(HIN* hin, string filePath, int startNodeIndex, int endNodeIndex, int linkIndex){
			int max=maxIndex(startNodeIndex, endNodeIndex, linkIndex), linkIndex1=endNodeIndex+1;
//			文件流
			ifstream fileStream(filePath);
			string line;
			getline(fileStream, line);
//			string流 
			istringstream stringStream(line);
			string temp;
//			title
			while(getline(stringStream, temp, ',')){
				title.push_back(temp); 
			}
			
//			add type
			hin->addNodeType(title[startNodeIndex]);
			hin->addNodeType(title[endNodeIndex]);
//			考虑边无权值情况
			if(linkIndex != -1){
				hin->addLinkType(title[startNodeIndex], title[endNodeIndex], title[linkIndex]);	
			}else{
//				linkIndex1++;
//				cout << "linkIndex changed:" << linkIndex1 << endl;
				hin->addLinkType(title[startNodeIndex], title[endNodeIndex], title[linkIndex1]);
			}
			
//			add value
			while(getline(fileStream, line)){
				istringstream stringStream(line);
				vector<string> temp1;
				string temp2;
				for(int i = 0; i <= max; i++){
					getline(stringStream, temp2, ',');
					temp1.push_back(temp2);
				}
//				add node
				hin->addNode(title[startNodeIndex], temp1[startNodeIndex]);
				hin->addNode(title[endNodeIndex], temp1[endNodeIndex]);
//				add link
				double linkValue;
				if(linkIndex != -1){
					linkValue = stod(temp1[linkIndex]);	
					hin->addLink(title[linkIndex], temp1[startNodeIndex], temp1[endNodeIndex], linkValue);
				}else{
					linkValue = 1;
					hin->addLink(title[linkIndex1], temp1[startNodeIndex], temp1[endNodeIndex], linkValue);
				}
			}
		};
};

#endif
