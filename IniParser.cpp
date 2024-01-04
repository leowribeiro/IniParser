
/*
------------------------------------------------------------

IniParser v1.2 - a *.ini file reader for C++
04/01/2024
Leonardo W. Ribeiro

------------------------------------------------------------
*/


#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cctype>
#include <vector>

class IniParser {
	
	private:
			
		std::map<std::string, std::map<std::string, std::string>> internal;
		std::string fileName;
		std::ifstream in;
		std::vector<std::string> tokens;
		
		std::string getToken();		
		bool reserved(unsigned char c);
		bool isIdent(std::string &str);
		std::string trimWS(std::string& str);
		void parse();
		
	public:

		class FileNotFound : public std::exception {};
		class SyntaxError : public std::exception {
			
			public:
				std::string message;
			
				SyntaxError(std::string message){
					this->message = message;
				}
							
				const char* what() const noexcept override {
					return message.c_str();
				}
			
		};
		class EOFReached : public std::exception {};
	
		IniParser(std::string fileName = ""){
			this->fileName = fileName;
		}
	
		std::map<std::string, std::string>& operator[](std::string section){
			return internal[section];
		}
	
		void clear(){
			internal.clear();
		}
		
		void read();
		void print();
};


bool IniParser::reserved(unsigned char c){
	
	if(  (c == '[') || (c == ']') || (c == '=') || (c == ';') || ( c == '\n') )
		return true;
	return false;
	
}

std::string IniParser::trimWS(std::string& str) {
	
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");

    if (first == std::string::npos || last == std::string::npos)
        return "";  // String contains only whitespaces

    return str.substr(first, last - first + 1);
}

std::string IniParser::getToken(){
	
	std::string t = "";
	
	unsigned char peeked = in.peek();

	if( in.eof() ){
		return "\0";
	}
	else if( reserved(peeked) ){
		t += in.get();
	}
	else {
	
		while( (!reserved(peeked)) && (!in.eof()) ){
			t += in.get();
			peeked = in.peek();
		}
		
	}
	
	if(t == "\n"){
		return t;
	}
	
	return trimWS(t);
}

void IniParser::read(){
		
	in.exceptions( std::ifstream::failbit | std::ifstream::badbit );
	
	try {
		in.open(fileName);
	}
	catch(...){
		throw IniParser::FileNotFound();
	}
	
	while( !in.eof() ){
		
		std::string token = getToken();
		
		if( !(token == "\0") )
			tokens.push_back(token);
	}
	
	in.close();
	
	parse();
}

bool IniParser::isIdent(std::string &str){
	
	if( str.length() == 1 ){
		if( reserved( str[0] ) )
			return false;
	}
	
	return true;
}

void IniParser::parse(){
		
	
	std::string curSection;
	int state = 0;
	
	int i = 0;
	int lineNumber = 1;
	
	for(i=0; i<tokens.size(); i++){
			
		std::string token = tokens[i];
	
		if( (state == 0) && (token == "[") ) {state = 1;}
		else if( (state == 1) && isIdent(token) ) {state = 2;}
		else if( (state == 2) && (token == "]") ){
			state = 0;
			curSection = tokens[i-1];
		}
		else if( (state == 0) && isIdent(token) ) {state = 3;}
		else if( (state == 3) && (token == "=") ) {state = 4;}
		else if( (state == 4) && isIdent(token) ){
			state = 0;
			internal[curSection][tokens[i-2]] = token;
		}
		else if( (state == 4) && (token == "\n") ){
			state = 0;
			internal[curSection][tokens[i-2]] = "";
			lineNumber++;
		}
		else if( (state == 0) && (token == ";") ) {state = 5;}
		else if( (state == 5) && isIdent(token) ) {state = 0;}
		else if( (state == 0) && (token == "\n") ) {
			state = 0;
			lineNumber++;
		}
		else {
			std::string message = "Syntax error on file \"" + fileName + "\" at line ";
			
			message += std::to_string(lineNumber);
			message += ".\n...";
				
			if( (i-1) >= 0 )
				message += tokens[i-1];
			message += " ->";
			if(token != "\n")
				message += token;
			else
				message += "LF";
			message += "<- ";
			if( (i+1) < tokens.size())
				message += tokens[i+1];
			
			message += "...";
			
			throw SyntaxError(message);
		}
	}

	if( state != 0 ){
		
		std::string message = "Syntax error on file \"" + fileName + "\" at line ";
		message += std::to_string(lineNumber);
		message += ".\n...";
		
		if( (i-1) >= 0 )
			message += tokens[i-1];
		message += " ->";
		if(tokens[i] != "\n")
			message += tokens[i];
		else
			message += "LF";
		message += "<- EOF";
		
		throw SyntaxError(message);
	}

}

void IniParser::print(){
	for (const auto& secPair : internal) {
		for (const auto& keyPair : secPair.second) {
			std::cout << "[" << secPair.first << "][" << keyPair.first << "]=" << keyPair.second << std::endl;
		}
    }
}

/*
int main(){
	
	IniParser config("config.ini");
	config.read();
	config.print();
	
	return 0;
}
*/


