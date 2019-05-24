
#include <cstdio>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

int size; //size of sudoku
int clausesCount; //number of clauses
int numVariables; //number of variables
vector< vector<int> > sud_arr;5
map<int,int> toCnf;
map<int,int> cnfTo;

//to display sudoku
void dispSudoku(void){
	for(int i=0;i<size;i++){
		for(int j=0;j<size;j++){
			cout << sud_arr[i][j] << " ";
 		}
 		cout << endl;
	}
}	

//to reset the given vector of size sz
void vecReset(vector<bool> &vec,int sz){
	for(int i=0;i<sz;i++){
		vec[i] = false;
	}
}

//to check if input string is a number
bool is_number(const string &s){
	string::const_iterator it = s.begin();
	while(it != s.end() && isdigit(*it)){
		++it;
	}
	return !s.empty() && it == s.end();
}

//to read cnf data from file
void readCnf(string fname){
	ifstream cnf(fname);
	string cnf_line;
	// getline(cnf,line);
	// string word;
	// istringstream iss(line);
	// bool size_read = false;
	// string::size_type sz;
	// //reading the first line
	// while(line >> word ){
	// 	int temp = stoi(word,&sz);
	// 	if(sz > 0){
	// 		if(size_read){
	// 			numClauses = stoi(word);
	// 		}else{
	// 			size = stoi(word);
	// 			size_read = true;
	// 		}
	// 	} 
	// }
	//reading the remaining lines ( clauses )
	int i=0; bool size_check = false;
	while(cnf >> cnf_line){
		vector<int> row_vec;
		istringstream iss_clause(cnf_line);
		int num;
		char c;
		//cout << cnf_line << endl;
		while( iss_clause >> c ){
			if(c == '.'){
				row_vec.push_back(0);
			}
			else{
				row_vec.push_back(c-'0');
			}
			//cout << "err" << endl;
			i++;
		}
		sud_arr.push_back(row_vec);
		//cout  << i << endl;
		if(!size_check){
			size = i;
			size_check = true;
		}
		i = 0;
	}


}


//to find variable number for row i, column j, number k
int num(int i,int j,int k,bool pos){
	int val = i*size*size + j*size + k + 1;
	if(!pos)
		return -val;
	return val;
}

//to find the number of clauses for given sudoku
int numClauses(){
	return size*size + (size * size ) * (size * (size - 1 ) ) / 2 + 2*size*size + 2*size;
}

void exec_fn(void){
	execl("/usr/bin/minisat","minisat","cnf.txt","output.txt",(char *)0);
	exit(0);
}


//to apply minisat on cnf file and process output
void applyMiniSat(void ){
	cout << "starting execl" << endl;
	int pid = fork();	
	if( pid == 0)		
		exec_fn();
	else
		wait(NULL);	

	ifstream op_file("output.txt");
		
	string output_text;
	int read_int,col=0;
	getline(op_file,output_text);
		
	if(output_text != string("SAT")  )
	{
		cout << " Unsatisfiable " << endl;
	}
	//satisfiable
	cout << "Satisfiable with following arrangement  :: " << endl;
	int p,q;
	while(op_file){
		op_file >> read_int;
		int d = read_int;
		//cout << read_int << endl;
		if(read_int == 0){
			return;
		}
		else if(d > 0){
			d--; //subtracting the one which we added to handle 0 at end
			p = d / (size *size);
			d = d % (size *size);
			q = d / size;
			d = d % size + 1 ;
			
			sud_arr[p][q] = d;
			cout << "["<<p<<"]" << "["<<q<<"] = " << d << "\n" ;
			col++;
			if(col == size){
				cout << endl;
				col = 0;
			}
		}
	}

	op_file.close();
}


int main(int argc,char **argv){
	
	if(argc != 3){
		cerr  << " Invalid arguments  " << endl;
		return -1;
	}
	
	//read sudoku size, input file
	string ip_f_name =  argv[2];
	//ifstream sudoku_file(ip_f_name);

	size = atoi(argv[1]);
	if(size <= 3){
		cerr <<  "Sudoku SOlver+ is not applicable for size : " << size << endl;
	//	return -1;
	}

	string cnf_f_name = "cnf.txt";	
	readCnf(ip_f_name);

	

	FILE* ip_file = fopen(cnf_f_name.c_str(),"w");
	int i,j,k,k1,k2,n = size;

	//push number of variables and cluases
	//string first_line  = "p cnf " +  to_string(size*size*size) + " " + to_string(numClauses()) + " \n";  
	//fputs(first_line.c_str(),ip_file);
	
	cout << " Constraint 1 doing" << endl; 
	//constraint 1 => Each cell contains atleast one copy of any number
	//n^2 clauses
	string clause = "";
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			//if the cell is already filled
			if(sud_arr[i][j] != 0)
				continue;
			clause = "";
			//if the cell is not filled
			//create the clause
			for(k=0;k<n;k++){
				clause = clause + " " +   to_string(num(i,j,k,true)) ;
			}
			clause = clause + " 0\n";
			//push the clause
			fputs(clause.c_str(),ip_file);
			clausesCount++;
		}
	}

	cout << " Constraint 2 doing " << endl; 
	//constraint 2 => Each cell contains atmost one copy of any number
	//n*n*(nC2) clauses
	clause = "";
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			//cell is already filled
			if(sud_arr[i][j] != 0)
				continue;
			//cell is not already filled
			for(k1 = 0;k1 < n;k1++){
				for(k2=k1+1;k2<n;k2++){
					//create the clause
					clause = to_string(num(i,j,k1,false)) + " " +  to_string(num(i,j,k2,false)) + " 0\n";
					//push the cluase
					fputs(clause.c_str(),ip_file);
					clausesCount++;		
				}
			}
		}
	}
	
	//fputs("contraint 3_start",ip_file);
	cout << " Constraint 3 doing " << endl;
	vector<bool> visit(size,false);
	vector<bool> visitNum(size,false);
	//contraint 3 => Each row should contain each number atleast once
	//n*n clauses
	for(i=0;i<n;i++){ //row
		vecReset(visit,size);
		vecReset(visitNum,size);
		for(int c=0;c<size;c++)
			if(sud_arr[i][c] > 0)
			{
				visitNum[sud_arr[i][c]-1] = true;
				visit[c] = true;
			}
		for(k=0;k<n;k++){ //number
			if(visitNum[k])
				continue;
			//create the clause 
			clause = "";
			for(j=0;j<n ;j++){
				if(visit[j])
					continue;
				clause = clause +  to_string(num(i,j,k,true)) + " ";
			}
			clause = clause + " 0\n";
			//push the clause
			fputs(clause.c_str(),ip_file);
			clausesCount++;
		}
	}

	//fputs("contraint 4_start",ip_file);
	
	//constraint 4 => Each column should contain each number atleast once
	//n*n clauses
	for(j=0;j<n;j++){ //column
		vecReset(visit,size);
		vecReset(visitNum,size);
		for(int r=0;r<size;r++)
			if(sud_arr[r][j] > 0)
			{
				visitNum[sud_arr[r][j]-1] = true;
				visit[r] = true;
			}
		for(k=0;k<n;k++) {//number
			if(visitNum[k])
				continue;
			//create the clause
			clause = "";
			for(i=0;i<n ;i++){
				if(visit[i])
					continue;
				clause = clause + to_string( num(i,j,k,true)) + " ";
			}
			clause = clause + " 0\n";
			//push the clause
			fputs(clause.c_str(),ip_file);
			clausesCount++;
		}
	//	fputs("\n",ip_file);
	}

	//fputs("contraint 5_start",ip_file);
	vecReset(visit,size);
	vecReset(visitNum,size);
	//constraint 5 => Each diagnol should contain each number atleast once
	//2*n clauses
	//diagnol 1
	for(int d=0;d<size;d++){
		if(sud_arr[d][d] > 0){
			visit[d] = true;
			visitNum[sud_arr[d][d]-1] = true;
		}
	}
	for(k=0;k<n ;k++){ //number
		if(visitNum[k])
			continue;	
		//creating clause		
		clause = "";			
		for(i=0;i<n ;i++){//diagnol entries
			if(visit[i])
				continue;
			clause = clause + to_string(num(i,i,k,true)) + " ";
		}
		clause = clause + " 0\n";
		//inserting the cluase
		fputs(clause.c_str(),ip_file);
		clausesCount++;	
	}
	vecReset(visit,size);
	vecReset(visitNum,size);
	for(int d=0;d<size;d++){
		if(sud_arr[d][size-d-1] > 0){
			visit[d] = true;
			visitNum[sud_arr[d][size-d-1]-1] = true;
		}
	}
	//diagnol 2
	for(k=0;k<n ;k++){ //number		
		if(visitNum[k])
			continue;
		//push clause		
		clause = "";			
		for(i=0;i<n ;i++){//dignol entries
			if(visit[i])
				continue;
			clause = clause + to_string(num(i,n-1-i,k,true)) + " ";
		}
		clause = clause + " 0\n";
		//push the clause
		fputs(clause.c_str(),ip_file);
		clausesCount++;
	}
	fclose(ip_file);
	
	cout << "Number of Clauses  : " << clausesCount << endl;
	cnfFinalise();
	dispSudoku();
	
	//applying minisat on dimacs to get solution
	applyMiniSat();
	dispSudoku();
	return 0;
}

