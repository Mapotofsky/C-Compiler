#include "symbol.h"

int funcnum = 0;				//符号表中元素个数

								//判断函数是否已经存在 并返回在该函数在函数名表中的位置 不存在则返回-1
int func_exist(struct func functable[255], int funcnum, string str)
{
	int i;
	for (i = 0;i < funcnum;i++)
		if (functable[i].name == str)
			return i;
	return -1;
}

//判断变量是否出现在当前函数的符号表中 存在则返回表中位置 否则返回-1
int sym_exist(struct func functable[255], struct symbol *st, int num, string str)
{
	for (int i = 0; i < functable[num].num; i++)
		if ((st + i)->name == str)
			return i;
	return -1;
}

// 获得当前变量的类型
string getType(string var, int num){
	for(int i=0; i<functable[num].num; i++) // 遍历当前函数符号表，找到变量的类型
		if((functable[num].st+i)->name == var)
			return (functable[num].st+i)->type;
}

// 获得当前变量的属性
string getProperty(string var, int num){
	for(int i=0; i<functable[num].num; i++) // 遍历当前函数符号表，找到变量的属性
		if((functable[num].st+i)->name == var)
			return (functable[num].st+i)->property;
}

//将当前函数名加入到当前函数表中
void pushsym(struct func functable[255], struct symbol *st, int curnum, int num) // curnum:当前函数编号
{
	st[functable[curnum].num].name = functable[num].name;
	st[functable[curnum].num].property = "IDF";
	st[functable[curnum].num].type = "-";
	st[functable[curnum].num].st = functable[num].st;
	functable[curnum].num++;
}

//将当前变量加入到当前函数表中
void pushsym2(struct func functable[255], struct symbol *st, int curnum, int num, string name, string property, string type)
{
	st[functable[curnum].num].name = name;
	st[functable[curnum].num].property = property;
	st[functable[curnum].num].type = type;
	functable[curnum].num++;
}

//将当前函数加入到函数表中 并返回该函数符号表的地址
struct symbol * pushfunc(struct func functable[255], int &funcnum, string str, string ret)
{
	functable[funcnum].name = str;
	functable[funcnum].num = 0;
	functable[funcnum].ret = ret;
	funcnum++;
	return functable[funcnum - 1].st;
}

//初始化符号表
void init_symtable(void)
{
	pushfunc(functable, funcnum, "input", "-");
	pushfunc(functable, funcnum, "output", "-");
}

//输出符号表
void output(fstream &out)
{
	int i, j;
	int set = 20;
	//输出函数符号表
	out << setw(set) << left << "函数名" << setw(set) << "函数中参数个数" << setw(set) << "函数返回类型" << setw(set) << "函数符号表地址" << endl;
	for (i = 0;i < funcnum;i++)
		out << setw(set - 1) << left << functable[i].name << setw(set - 2) << functable[i].num << setw(set - 2) << functable[i].ret << "0x" << setw(set) << functable[i].st << endl;
	//输处各函数表
	for (i = 0;i < funcnum;i++)
	{
		out << endl;
		out << "函数名:" << functable[i].name << "\t符号表地址:0x" << functable[i].st << endl;
		if (functable[i].num)
			out << setw(set) << "变量名" << setw(set) << "属性" << setw(set) << "类型" << setw(set) << "地址" << endl;
		for (j = 0;j < functable[i].num;j++)
			out << setw(set - 1) << functable[i].st[j].name << setw(set - 1) << functable[i].st[j].property << setw(set - 1) << functable[i].st[j].type << setw(set) << functable[i].st[j].st << endl;
	}
}

//生成符号表
void calsymboltable(fstream &in, fstream &out)
{
	vector<Token> tokens;
	Token token;
	struct symbol *cur = NULL;
	int flag1, flag2, num, curnum;
	//读入词法分析结果
	while (!in.eof())
	{
		in >> token.re >> token.type >> token.line >> token.colume;
		if (in.fail())
			break;
		tokens.push_back(token);
	}
	flag1 = 0; // 大括号标记
	flag2 = 0; // 小括号标记
	init_symtable();


	//检测当前token的属性为函数名或者变量名，根据标记括号的flag1.flag2判断位置 
	for (vector<Token>::iterator titer = tokens.begin();titer != tokens.end();titer++)
	{
		if (titer->type == "IDF")	//当前token为函数名,分为函数的定义和调用情况 
		{
			num = func_exist(functable, funcnum, titer->re);
			if (flag1 == 0)//之前不存在左大括号，说明当前为函数的定义
			{
				if (num != -1)//已存在该函数名
				{
					cur = functable[num].st;  //得到该函数名在符号表中的信息
					curnum = num;
				}
				else     //不存在该函数名,写入该函数名 
				{
					cur = pushfunc(functable, funcnum, titer->re, (titer - 1)->re);
					curnum = funcnum - 1;
				}
			}
			else if (flag1 > 0)   //说明是函数内部对函数的调用
			{
				if (num == -1)	//函数符号表中未出现 函数没有被定义就被调用 报错
				{
					cout << "Undefined Function [" << titer->re << "] at Line " << titer->line << " Colume " << titer->colume << endl;
					exit(0);
				}
				else//已存在该函数名，判断是否为首次调用 是则加入至当前函数的变量表中
				{
					if (sym_exist(functable, cur, curnum, titer->re) == -1)
						pushsym(functable, cur, curnum, num);
				}
			}
		}

		else if (titer->type == "ID")											//当前token为变量名，分为变量的声明和调用情况 
		{
			if ((titer - 1)->re == "void" || (titer - 1)->re == "int" || (titer - 1)->re == "char")		//前一个token为int或void，说明当前为变量的声明 
			{
				if (flag1 == 0 || (flag1 > 0 && flag2 == 0)) 				//变量声明只能在函数获取形参中或不在其他位于函数内部的小括号内 
				{
					if (sym_exist(functable, cur, curnum, titer->re) == -1) 	//不存在该变量，则定义，入表 
						if((titer+1)->re == "[") // 如果变量名后是左括号，则说明是数组定义
							pushsym2(functable, cur, curnum, num, titer->re, "ARRAY", (titer - 1)->re);
						else  // 变量定义
							pushsym2(functable, cur, curnum, num, titer->re, "VARIABLE", (titer - 1)->re);
					else														//存在该变量，不用入表 
					{
						cout << "Variable [" << titer->re << "] has been defined!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "//变量定义在错误的位置 ";
				}
			}
			else															//前一个token不是int或void，说明为变量的调用 
			{
				if (sym_exist(functable, cur, curnum, titer->re) == -1)			//符号表中不存在，说明未定义就使用
				{
					cout << "Undefined Variable [" << titer->re << "] at Line " << titer->line << " Colume " << titer->colume << endl;
					exit(0);
				}
			}
		}
		else if(titer->type == "OOP"){ // 若是运算符（OOP），判断前后操作数类型是否相同，是否是该运算符支持的类型
			// 检查左操作数token类型
			string pre_type;
			if((titer-1)->re == "]"){ // 若前一个token是右括号，则说明左操作数是数组中的元素
				vector<Token>::iterator it;
				// 向前遍历token，直到找到"[",其前一个就是目标token
				for(it = titer-2; it->re!="[" ;it--)
					continue;
				it--;
				// 查符号表得到其类型
				pre_type = getType(it->re, curnum);
			}
			else
				pre_type= ((titer-1)->type == "NUM" ? "int" : getType((titer-1)->re, curnum));
			if(pre_type == "void" || (pre_type == "char" && titer->re != "+")){
				out << "Grammar Error at Line " << (titer-1)->line << " Colume " << (titer-1)->colume << endl;
				cout << "Grammar Error at Line " << (titer-1)->line << " Colume " << (titer-1)->colume << endl;
				exit(0);
			}
			// 检查右操作数token类型
			string next_type = ((titer+1)->type == "NUM" ? "int" : getType((titer+1)->re, curnum));
			if(pre_type == "void" || (pre_type == "char" && titer->re != "+") || pre_type != next_type){
				out << "Grammar Error at Line " << (titer+1)->line << " Colume " << (titer+1)->colume << endl;
				cout << "Grammar Error at Line " << (titer+1)->line << " Colume " << (titer+1)->colume << endl;
				exit(0);
			}
		}
		// 出现左中括号，判断是否是数组类型
		else if(titer->re == "["){
			string pre_property = getProperty((titer-1)->re, curnum);
			if(pre_property != "ARRAY"){ // 如果不是数组类型，报错
				out << "Grammar Error at Line " << titer->line << " Colume " << titer->colume << endl;
				cout << "Grammar Error at Line " << titer->line << " Colume " << titer->colume << endl;
				exit(0);
			}
		}
		else if (titer->re == "{")
			flag1++;
		else if (titer->re == "}")
			flag1--;
		else if (titer->re == "(")
			flag2++;
		else if (titer->re == ")")
			flag2--;
	}
	output(out);
}

