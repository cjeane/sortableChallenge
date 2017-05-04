#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

//a crude json parser based on sample files
class JParse{
public:
	bool containsTerm(std::string term, std::string line);
	std::string getValue(std::string term, std::string line);
	std::string stripeSpChars(std::string term);
};

bool JParse::containsTerm(std::string term, std::string line){
	term= "\""+term+"\":";
	return line.find(term)!=std::string::npos;
}

std::string JParse::stripeSpChars(std::string term){
	std::string str = term;
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
	str.erase(remove_if(str.begin(), str.end(), ispunct), str.end());
	return str;
}

std::string JParse::getValue(std::string term, std::string line){
	term= "\""+term+"\":";
	int pos = line.find(term);
	
	std::string sub = line.substr(pos);
	int s = sub.find(":") + pos;
	int f = line.substr(s).find(",\"");
	sub =line.substr(s+2,f-3);
	return sub;
}

int main(){
	bool flag;	
	std::ifstream fileProducts("products.txt");
	std::ifstream fileListing("listings.txt");
	std::ofstream outFile;
	
	std::string line;
	std::string word, temp;
	std::string rawTitle;
	std::string res;	
	
	std::map<std::string, std::set<std::string>> mapManufactor;
	std::map<std::string, std::vector<std::string>> answer;
	std::map<std::string, std::string> modelToProduct;
	JParse parse;	

	while(std::getline(fileProducts, line)){
		
		word = parse.getValue("manufacturer",line);
		
		//builds map of manufactor to list of models
		if (mapManufactor.find(word)==mapManufactor.end()){
			std::transform(word.begin(), word.end(), word.begin(), ::tolower);		
			mapManufactor.insert(std::pair<std::string,std::set<std::string>>(word,std::set<std::string>()));			
		}
				
		temp = parse.stripeSpChars(parse.getValue("model",line));
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		mapManufactor.find(word)->second.insert(temp);
		
		//builds map of converted model name to original product name
		modelToProduct.insert(std::pair<std::string,std::string>(temp,parse.getValue("product_name",line)));
		
		//builds map of product name to matching listings
		answer.insert(std::pair<std::string, std::vector<std::string>>(parse.getValue("product_name",line)
				,std::vector<std::string>()));
		
	}
	
	while(std::getline(fileListing, line)){
		
		rawTitle = parse.stripeSpChars(parse.getValue("title",line));
		std::transform(rawTitle.begin(), rawTitle.end(), rawTitle.begin(), ::tolower);
		temp =  parse.stripeSpChars(parse.getValue("manufacturer",line));
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		flag=true;
		
		//tries finding a matching brand then a model of said brand based on title of listing
		for (auto brand : mapManufactor){			
			if (rawTitle.find(brand.first)!=std::string::npos){				
				for (auto model : mapManufactor.find(brand.first)->second){
					if (rawTitle.find(model)!=std::string::npos){						
						flag=false;																		
						auto iter = modelToProduct.find(model);
						answer.find(iter->second)->second.push_back(line);						
						break;
						
					}
				}
				break;
			}
		}
		
		//extra catch for some cases which doesn't have brand name in title
		if (flag && mapManufactor.find(temp)!=mapManufactor.end()){
			for (auto model : mapManufactor.find(temp)->second){
				if (rawTitle.find(model)!=std::string::npos){										
					auto iter = modelToProduct.find(model);
					answer.find(iter->second)->second.push_back(line);
					flag=false;					
					break;
				}
			}

		}
		
	}
	
	fileProducts.close();
	fileListing.close();

	outFile.open("results.txt");	
	
	//generate results json objects
	for (auto iter: answer){		
		if (iter.second.size()>0){
			temp="";			
			for(auto i: iter.second)
				temp+=i+",";							
			temp.pop_back();
			res = "{\"product_name\":\""+iter.first+"\",\"listings\":["+temp+"]}";						
		}
			
		else{			
			res = "{\"product_name\":\""+iter.first+"\",\"listings\":[]}";			
		}
			outFile<<res <<'\n';		
	}
	outFile.close();
	

}

