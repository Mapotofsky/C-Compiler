#include "grammar-analysis.h"

Token token;
struct Mtable mtable;
struct Stack stack;
treebit *treenode;

int nodenum = 0;		//节点编号
map<int, vector<string>> placelistmap;		//各节点存储字符串信息表

map<string, bool(*)(treebit *cur)> funcmap;		//推导式编号与其对应的语义动作函数
map<int, string> midcodemap;
map<string, int> funcnum;
int curnum = 99;		//三地址代码编号
int curtemp = 0;		//临时变量个数

string getVarType(string var){
	// 找到当前函数名
	string curFun;
	string part1, part2;
	for(int i=curnum; i>99; i--){
		string cur = midcodemap[i];
		int split;
		split = cur.find(" ");
		if(split != cur.npos && split != cur.length()-1){
			part1.assign(cur, 0, split+1);
			part2.assign(cur, split+1, cur.length() - split - 1);
			if(part1 == proc){
				curFun = part2;
				break;
			}
		}
	}
	
	// 遍历函数表，找到当前函数的符号表，并遍历该符号表，找到变量类型
	for(int i=0; i<255; i++){ // 遍历函数表
		if(functable[i].name == part2) // 若找到当前函数
			for(int j=0; j<functable[i].num; j++) // 遍历当前函数符号表
				if((functable[i].st+j)->name == var) // 找到变量类型
					return (functable[i].st+j)->type;
	}
}
//初始化终结符类型表
void init_vttype(map<string, string> &vttype)
{
	vttype["<"] = "COP";
	vttype["<="] = "COP";
	vttype[">"] = "COP";
	vttype[">="] = "COP";
	vttype["=="] = "COP";
	vttype["!="] = "COP";
	vttype["="] = "AOP";
	vttype["+"] = "OOP";
	vttype["-"] = "OOP";
	vttype["*"] = "OOP";
	vttype["/"] = "OOP";
	vttype[";"] = "EOP";
	vttype[","] = "SOP";
	vttype["("] = "SOP";
	vttype[")"] = "SOP";
	vttype["["] = "SOP";
	vttype["]"] = "SOP";
	vttype["{"] = "SOP";
	vttype["}"] = "SOP";
	vttype["int"] = "REVERSED";
	vttype["char"] = "REVERSED";
	vttype["if"] = "REVERSED";
	vttype["else"] = "REVERSED";
	vttype["return"] = "REVERSED";
	vttype["void"] = "REVERSED";
	vttype["while"] = "REVERSED";
	vttype["NUM"] = "NUM";
	vttype["ID"] = "ID";
	vttype["IDF"] = "IDF";
	vttype["ID1"] = "ID1";
	vttype["empty"] = "empty";
	vttype["#"] = "#";
}

//初始化预测分析表mtable
void init_mtable(struct Mtable &mtable)
{
	int i, j, vnnum, vtnum;
	for (iter = vn.begin(), vnnum = 1;iter != vn.end();iter++, vnnum++)
		mtable.vnname.insert(make_pair(*iter,vnnum));
	//预测分析表中的终结符部分需要去除empty 产生空字的列
	for (iter = vt.begin(), vtnum = 1;iter != vt.end();iter++, vtnum++)
		if (*iter != "empty")
			mtable.vtname.insert(make_pair(*iter,vtnum));
		else
			vtnum--;
	//初始化全赋值为0 推导式编号由1开始
	for (i = 1;i < vnnum;i++)
		for (j = 1;j < vtnum;j++)
			mtable.mtable[i][j] = 0;
	mtable.vnnum = vnnum - 1;
	mtable.vtnum = vtnum - 1;
}

//判断集合中是否存在str元素
int check_exist(vector<string> vec, string str)
{
	vector<string>::iterator iter;
	iter = find(vec.begin(), vec.end(), str);
	if (iter != vec.end())
		return 1;
	else
		return 0;
}

//判断vtmap中是否有该vt
int checkmap(map<string, string> mp, string str)
{
	iter_map = vttype.find(str);
	if (iter_map != vttype.end())
		return 1;
	else
		return 0;
}

//删除vt集中应该为vn的元素
void del(vector<string> &vec, string str)
{
	iter = find(vec.begin(), vec.end(), str);
	vec.erase(iter);
}

//计算终结符集合以及非终结符集合
void cal_vn_vt(fstream &in,fstream &out)
{
	string temp,part1,part2,t;
	int split, start, index, num = 1; // num表示产生式编号
	init_vttype(vttype);
	while (!in.eof())
	{
		getline(in, temp); // 每次循环读取一条产生式
		if (in.fail())
			break;
		temp.replace(0, temp.find(' '), "");	//去掉每行开头的数字
		split = temp.find("->");
		part1.assign(temp, 0, split);
		part2.assign(temp, split + 2, temp.length() - split - 1);
		fix(part1);
		fix(part2);			//将每一行语法分为箭头前后的两个部分
		start = 0;
		vector<string>* temp1 = new vector<string>;
		temp1->push_back(part1);
		grammarmap.insert(make_pair(num, *temp1));
		//左侧部分 如果vn集合中还没有出现则加入 如果vt集合中出现则将vt集合中的该元素删除
		if (!check_exist(vn, part1))
			vn.push_back(part1);
		if (check_exist(vt, part1))
			del(vt, part1);
		while (1)		//将箭头后的部分按照空格分开
		{
			index = part2.find(' ',start);
			if (index == part2.npos)
				break;
			t.assign(part2, start, index - start);
			start = index + 1;
			//当右侧部分在 vn中没有出现且vt中没有出现 则将该元素加入vt集合中
			if (!check_exist(vn, t) && !check_exist(vt, t))
				vt.push_back(t);
			grammarmap[num].push_back(t);
		}
		// 对右侧最后一个符号操作
		t.assign(part2, start, part2.length() - start);
		if (!check_exist(vn, t) && !check_exist(vt, t))
			vt.push_back(t);
		//填充可以推导出空的非终结符表
		grammarmap[num].push_back(t);
		if (grammarmap[num][1] == "empty")
			emptymake.insert(make_pair(grammarmap[num][0],num));
		num++;
	}
	vt.push_back("#");
	//输出
	output_vec(out);
}

//输出vector中所有信息 输出vn,vt信息至文件
void output_vec(fstream &out)
{
	out << "非终结符vn:" << endl;
	for (iter = vn.begin();iter != vn.end();iter++)
	{
		out << *iter << endl;
		vector<string>* temp1 = new vector<string>;
		vector<string>* temp2 = new vector<string>;
		firstlist.insert(make_pair(*iter, *temp1));
		followlist.insert(make_pair(*iter, *temp2));
		if (*iter == firststr)
			followlist[firststr].push_back("#");
	}
	out << "终结符vt:" << endl;
	for (iter = vt.begin();iter != vt.end();iter++)
	{
		if (!checkmap(vttype, *iter))
		{
			//cout << "Undefined vt[ " << *iter << " ]!" << endl;
			//exit(0);
			;
		}
		out << *iter << endl;
		vector<string>* temp_vec = new vector<string>;
		temp_vec->push_back(*iter);
		firstlist.insert(make_pair(*iter, *temp_vec));
	}
}

//求first集
void cal_first()
{
	int flag; // 标志一轮循环中是否对于所有的first集合没有进行改变
	int outflag; 
	unsigned int k;
	string vnname, tname, temp;
	while (1)
	{
		flag = 1; 
		for (gmaper = grammarmap.begin();gmaper != grammarmap.end();gmaper++)
		{
			vnname = gmaper->second[0]; // 产生式左侧的非终结符
			if (check_exist(vt, gmaper->second[1])) // 产生式右侧第一个元素为终结符
			{
				if (!check_exist(firstlist[vnname], gmaper->second[1])) // 判断该终结符是否已经在该推导的first集中,没有则添加
				{
					firstlist[vnname].push_back(gmaper->second[1]);
					flag = 0; // 表示first集发生变化
				}	
			}		
			else // 产生式右侧第一个元素为非终结符
			{
				outflag = 1;
				for(unsigned int k=1; k< gmaper->second.size(); k++){
					//cout << k << " " << gmaper->second.size() << endl;
					tname = gmaper->second[k];
					if (check_exist(vn, tname))
					{
						//逐个检验是否已经在vnname的first集中,不存在就添加
						for (iter = firstlist[tname].begin();iter != firstlist[tname].end();iter++)
						{
							temp = *iter;
							if (!check_exist(firstlist[vnname], temp))
							{
								firstlist[vnname].push_back(temp);
								flag = 0;
							}
							if (*iter == "empty")	//如果该非终结符的first集中没有empty则进入下一推导式
								outflag = 0;
						}
					}
					else
					{		//推导式中出现终结符 判断是否添加后 进入下一个推导式
						outflag = 1;
						if (!check_exist(firstlist[vnname], tname))
						{
							firstlist[vnname].push_back(tname);
							flag = 0;
						}
					}
					if (outflag)
						break;
				}
			}
		}
		//当flag为1时表示已经没有first集发生添加元素 fisrt集合求解完成
		if (flag)
			break;
	}
}

//求follow集
void cal_follow()
{
	int flag, outflag, exflag;
	string vnname, tname;
	vector<string>::iterator titer, temp;
	while (1)
	{
		flag = 1;
		for (gmaper = grammarmap.begin();gmaper != grammarmap.end();gmaper++)		//遍历推导式
		{
			for (iter = gmaper->second.begin() + 1;iter != gmaper->second.end();iter=titer)		//遍历推导式中个符号
			{
				titer = iter + 1; // 保存下一个文法符号的位置
				if (check_exist(vt, *iter))		//遇到终结符 不需要计算follow集
					continue;
				else  // 遇到一个非终结符开始向后遍历 直到遇到不能推导空的非终结符或是终结符
				{
					vnname = *iter;		
					while (1) // 遍历当前符号后的所有符号
					{
						iter++;
						outflag = 1;		//记录一个非终结符能不能推导出空
						if (iter == gmaper->second.end())		//到推导式结束 当前非终结符follow集操作完成
							break;
						if (check_exist(vt, *iter))  // 遇到一个终结符 判断是否存在 不存在则加入当前非终结符的follow集中
						{
							if (!check_exist(followlist[vnname], *iter) && *iter != "empty")
							{
								followlist[vnname].push_back(*iter);
								flag = 0;
							}
						}		
						else // 将当前符号后遇到的所有的first集合判断并加入到follow集中 直到遇到不能推导出空的非终结符
						{
							for (temp = firstlist[*iter].begin();temp != firstlist[*iter].end();temp++)
							{
								if (!check_exist(followlist[vnname], *temp) && *temp != "empty")
								{
									followlist[vnname].push_back(*temp);
									flag = 0;
								}
								if (*temp == "empty")
									outflag = 0;
							}		
						}
						if (outflag)
							break;
					}
				}
			}
			unsigned int i = 1;
			//推导式最后连续i个非终结符可以推导为空 则推导式最前的非终结符的follow集中所有元素判断并添加到这所有的非终结符的follow集中
			while (1)
			{			
				if (check_exist(vn, *(gmaper->second.end() - i)))
				{
					string str;
					if (i >= gmaper->second.size())
						break;
					str = *(gmaper->second.end() - i); // 当前非终结符
					vnname = gmaper->second[0]; // 产生式左侧非终结符
					// 遍历当前产生式左侧的follow集，全部加入到当前非终结符的follow集中
					for (temp = followlist[vnname].begin();temp != followlist[vnname].end();temp++)
					{
						if (!check_exist(followlist[str], *temp) && *temp != "empty")
						{
							followlist[str].push_back(*temp);
							flag = 0;
						}
					}
					exflag = 0;
					for (iter = firstlist[str].begin();iter != firstlist[str].end();iter++)
						if (*iter == "empty")
							exflag = 1;
					//exflag记录该非终结符的first集中是否有empty
					if (!exflag)
						break;
					else
						i++;
				}
				else
					break;
			}
		}
		if (flag)
			break;
	}
}

//计算first follow集
void cal_first_follow(fstream &in, fstream &out)
{
	cal_first();
	cal_follow();
	output_ff(out);
}

//输出first,follow集至文件
void output_ff(fstream &out)
{
	//输出first集
	out << "//first集" << endl;
	for (maper = firstlist.begin();maper != firstlist.end();maper++)
	{
		if (!check_exist(vt, maper->first))
		{
			out << maper->first << "的first集:";
			for (iter = maper->second.begin();iter != maper->second.end();iter++)
				out << *iter << " ";
			out << endl;
		}
	}
	//输出follow集
	out << "//follow集" << endl;
	for (maper = followlist.begin();maper != followlist.end();maper++)
	{
		if (!check_exist(vt, maper->first))
		{
			out << maper->first << "的follow集:";
			for (iter = maper->second.begin();iter != maper->second.end();iter++)
				out << *iter << " ";
			out << endl;
		}
	}
}

//计算预测分析表
void cal_mtable(fstream &in, fstream &out)
{
	string name, vnname;
	int emptyflag, i, j;
	init_mtable(mtable);
	//首先输出终结符非终结符个数
	out << "vnnum: " << mtable.vnnum << endl;
	out << "vtnum: " << mtable.vtnum << endl;
	for (gmaper = grammarmap.begin();gmaper != grammarmap.end();gmaper++)
	{
		name = gmaper->second[1];		//当前推导式箭头右侧第一个符号
		vnname = gmaper->second[0];		//当前推导式箭头左侧的非终结符
		emptyflag = 0;
		for (iter = firstlist[name].begin();iter != firstlist[name].end();iter++)
		{
			if (*iter == "empty")
			{
				emptyflag = 1;
				continue;
			}
			//将name的所有first集所对应的位置填入当前推导式的编号
			mtable.mtable[mtable.vnname[vnname]][mtable.vtname[*iter]] = gmaper->first;
		}
		// 当右侧第一个符号可以推出空时，将第二个符号的first集所对应的位置填入当前产生式的编号
		if (name == "empty"&&gmaper->second.size() > 2)
		{
			name = gmaper->second[2];
			for (iter = firstlist[name].begin();iter != firstlist[name].end();iter++)
				mtable.mtable[mtable.vnname[vnname]][mtable.vtname[*iter]] = gmaper->first;
		}
		//如果name可以产生空字 则将vnname所有的follow集所对应的位置填上 vnname推导出空的编号
		if (emptyflag)
		{
			for (iter = followlist[vnname].begin();iter != followlist[vnname].end();iter++)
				mtable.mtable[mtable.vnname[vnname]][mtable.vtname[*iter]] = emptymake[vnname];
		}
	}
	//格式化输出预测分析表至文件
	out << setw(20) << "";
	for (iter = vt.begin();iter != vt.end();iter++)
		if (*iter != "empty")
			out << setw(8) << left << *iter;
	out << endl;
	for (iter = vn.begin(), i = 1;iter != vn.end();iter++, i++)
	{
		out << setw(20) << left << *iter;
		for (j = 1;j <= mtable.vtnum;j++)
			out << setw(8) << left << mtable.mtable[i][j];
		out << endl;
	}
}

//初始化栈
void stack_init(struct Stack &stack)
{
	stack.st.push_back("#");
	stack.st.push_back(firststr);
	stack.current = stack.st.size();
}

//元素入栈
void stack_pushin(struct Stack &stack, string str)
{
	stack.st.push_back(str);
	stack.current++;
}

//获取栈顶元素
string stack_gettop(struct Stack stack)
{
	return stack.st[stack.current - 1];
}

//栈顶元素出栈
void stack_popout(struct Stack &stack)
{
	string str;
	if (stack.st.size() == 0)
	{
		cout << "ERROR:栈已为空,无法出栈!" << endl;
		exit(0);
	}
	else
	{
		stack.st.pop_back();
		stack.current--;
	}
}

//输出栈中信息
void stack_show(struct Stack stack, fstream &out)
{
	int i;
	out << "当前栈中元素个数:" << stack.current;
	for (i = 0;i < stack.current;i++)
		out << " " << stack.st[i];
	out << endl;
}

//初始化语法树
void tree_init()
{
	treenode = (treebit *)malloc(sizeof(treebit));
	strcpy_s(treenode->re, "head");
	strcpy_s(treenode->type, "derivation");
	treenode->bitnum = 3;
	treenode->father = NULL;
	treenode->brother = NULL;		//初始化根节点 
	for (int i = 1;i <= treenode->bitnum;i++)
	{
		treenode->sons[i] = (struct Gtree *)malloc(sizeof(struct Gtree));
		treenode->sons[i]->father = (struct Gtree *)malloc(sizeof(struct Gtree));
		treenode->sons[i]->brother = (struct Gtree *)malloc(sizeof(struct Gtree));
	}
	strcpy_s(treenode->sons[1]->re, "#");
	strcpy_s(treenode->sons[1]->type, "#");
	treenode->sons[1]->brother = treenode->sons[2];
	treenode->sons[1]->father = treenode;
	nodenum++;
	treenode->sons[2]->nodenum = nodenum;
	vector<string>* temp1 = new vector<string>;
	placelistmap.insert(make_pair(nodenum, *temp1));
	strcpy_s(treenode->sons[2]->re, firststr.c_str());
	strcpy_s(treenode->sons[2]->type, "derivation");
	treenode->sons[2]->brother = treenode->sons[3];
	treenode->sons[2]->father = treenode;
	strcpy_s(treenode->sons[3]->re, "#");
	strcpy_s(treenode->sons[3]->type, "#");
	treenode->sons[3]->brother = NULL;
	treenode->sons[3]->father = treenode;		//初始化根节点的孩子节点 使用推导式 firststr-># firststr #
}

//生成语法分析树
void make_tree(fstream &in, fstream &out)
{
	string str;
	int num, line;
	struct Gtree *cur;
	tree_init();
	stack_init(stack);				//初始化语法分析树以及语法栈
	cur = treenode->sons[2];
	while (!in.eof()) //从词法分析结果中逐个读取token
	{
		if (!stack.current)
			break;
		in >> skipws;
		in >> token.re >> token.type >> token.line >> token.colume; 
		if (in.fail())
			break;				
		stack_show(stack,out);
		str = stack_gettop(stack);
		while (mtable.vtname.find(str) == mtable.vtname.end()) //直到栈顶元素为终结符 跳出循环 准备比较\出栈操作
		{	//数字 ID类型token通过原始字符串得到编号 其余通过type类型
			if (token.type != "NUM" && token.type != "ID" && token.type != "IDF" && token.type != "ID1")
				num = mtable.vtname[token.re];
			else
				num = mtable.vtname[token.type];		
			if (mtable.mtable[mtable.vnname[str]][num] == 0) //预测分析表行列所对应的推导式标号为0 表示出现语法错误
			{
				out << "Grammar Error at Line " << token.line << " Colume " << token.colume << endl;
				cout << "Grammar Error at Line " << token.line << " Colume " << token.colume << endl;
				//cout << str << endl;
				//cout << token.re << endl;
				exit(0);
			}		
			else // 查询预测分析表，没有出现错误，查到推导应使用产生式
			{
				line = mtable.mtable[mtable.vnname[str]][num]; // 查询预测分析表，得到产生式编号
				stack_popout(stack);
				cur->bitnum = 0;
				cur->mtnum = line;
				//将不为empty的产生式右侧部分  逆序压入栈中
				for (iter = grammarmap[line].end() - 1;iter != grammarmap[line].begin();iter--)
				{
					if (*iter != "empty")
					{
						stack_pushin(stack, *iter);
					}
				}		
				//将该条产生式的所有元素 加入到当前节点的孩子节点 并填充相关信息以及兄弟父亲节点
				for (iter = grammarmap[line].begin() + 1;iter != grammarmap[line].end();iter++)
				{
					if (cur->mtnum == 42)
					{
						if (*iter == "expression")
						{
							cur->bitnum++;
							cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
							cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
							cur->sons[cur->bitnum]->father = cur;
							strcpy_s(cur->sons[cur->bitnum]->re, "$42_1");
							strcpy_s(cur->sons[cur->bitnum]->type, "todo");
						}
						if (*iter == ")")
						{
							cur->bitnum++;
							cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
							cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
							cur->sons[cur->bitnum]->father = cur;
							strcpy_s(cur->sons[cur->bitnum]->re, "$42_2");
							strcpy_s(cur->sons[cur->bitnum]->type, "todo");
						}
						//42 iteration-stmt -> while ( $42_1 expression $42_2 ) statement $42_3
					}		//while语句推导中需要增加两个中间语义动作
					if (cur->mtnum == 39)
					{
						if (*iter == ")")
						{
							cur->bitnum++;
							cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
							cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
							cur->sons[cur->bitnum]->father = cur;
							strcpy_s(cur->sons[cur->bitnum]->re, "$39_1");
							strcpy_s(cur->sons[cur->bitnum]->type, "todo");
						}
						if (*iter == "selection-stmt'")
						{
							cur->bitnum++;
							cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
							cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
							cur->sons[cur->bitnum]->father = cur;
							strcpy_s(cur->sons[cur->bitnum]->re, "$39_2");
							strcpy_s(cur->sons[cur->bitnum]->type, "todo");
						}	//39 selection-stmt -> if ( expression $39_1 ) statement $39_2 selection-stmt' $39_3
					}		//if语句推导中需要增加两个中间语义动作
					if (cur->mtnum == 41)
					{
						if (*iter == "statement")
						{
							cur->bitnum++;
							cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
							cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
							cur->sons[cur->bitnum]->father = cur;
							strcpy_s(cur->sons[cur->bitnum]->re, "$41_1");
							strcpy_s(cur->sons[cur->bitnum]->type, "todo");
						}	//41 selection-stmt' -> else $41_1 statement $41_2
					}		//else语句推导中需要增加一个中间语义动作
					// 创建新的子节点并修改相关数据
					cur->bitnum++;
					cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
					if (cur->bitnum > 1)
						cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
					cur->sons[cur->bitnum]->father = cur;
					strcpy_s(cur->sons[cur->bitnum]->varType, "");
					strcpy_s(cur->sons[cur->bitnum]->re, (*iter).c_str());
					if (mtable.vnname.find(*iter) != mtable.vnname.end()) // 是非终结符，则节点类型为derivation
						strcpy_s(cur->sons[cur->bitnum]->type, "derivation");
					else  // 是终结符，类型为该终结符类型
						strcpy_s(cur->sons[cur->bitnum]->type, vttype.find(*iter)->second.c_str());
					cur->sons[cur->bitnum]->info = (struct info *)malloc(sizeof(struct info));
					// 修改节点编号
					nodenum++;
					cur->sons[cur->bitnum]->nodenum = nodenum;
					vector<string>* temp1 = new vector<string>;
					placelistmap.insert(make_pair(nodenum, *temp1));
				}		
				//最后添加$节点 表明用第几条产生式产生 便于后续语义分析 ；语义分析节点命名由 $ 符号 加上 产生式编号组成
				cur->bitnum++;
				stringstream ss;
				ss << cur->mtnum;
				cur->sons[cur->bitnum] = (treebit *)malloc(sizeof(treebit));
				strcpy_s(cur->sons[cur->bitnum]->re, ("$" + ss.str()).c_str());
				if (ss.str() == "42") // 对三条特殊产生式，进行变换操作
					strcpy_s(cur->sons[cur->bitnum]->re, "$42_3");
				if (ss.str() == "39")
					strcpy_s(cur->sons[cur->bitnum]->re, "$39_3");
				if (ss.str() == "41")
					strcpy_s(cur->sons[cur->bitnum]->re, "$41_2");
				strcpy_s(cur->sons[cur->bitnum]->type, "todo");
				cur->sons[cur->bitnum]->father = cur;
				cur->sons[cur->bitnum - 1]->brother = cur->sons[cur->bitnum];
				cur->sons[cur->bitnum]->brother = NULL;			
				cur = cur->sons[1]; // 进入第一个子节点
				//应用完当前产生式 进入兄弟节点继续推导 没有兄弟节点 返回父亲节点 进入它的兄弟节点 知道兄弟节点存在
				if (strcmp(cur->re, "empty") == 0)
				{
					while (1)
					{
						while (!cur->brother)
						{
							cur = cur->father;
							if (strcmp(cur->re, "head") == 0)
								break;
						}
						cur = cur->brother;
						if (cur->re[0] != '$')
							break;
					}
				}		
				str = stack_gettop(stack);
				stack_show(stack, out);
			 }
		}
		//栈顶终结符类型与当前token类型不符合 报错
		if (token.re != stack_gettop(stack) && token.type != stack_gettop(stack))
		{
			out << "Grammar Error at Line " << token.line << " Colume " << token.colume << endl;
			cout << "Grammar Error at Line " << token.line << " Colume " << token.colume << endl;
			exit(0);
		}				
		if (strstr(cur->type, "ID"))
			strcpy_s(cur->re, token.re.c_str());		//对于id类型直接改变当前节点原始字符串 不再产生新的节点
		if (strstr(cur->type, "NUM"))
			strcpy_s(cur->re, token.re.c_str());
		//寻找满足条件的兄弟节点
		while (1)
		{
			while (!cur->brother)
			{
				cur = cur->father;
				if (strcmp(cur->re, "head") == 0)
					break;
			}
			if (strcmp(cur->re, "head") == 0)
				break;
			cur = cur->brother;	
			if (cur->re[0] != '$')
				break;
		}		
		//栈顶元素出栈
		stack_popout(stack);
	}
}

//产生一个新的临时变量
string newtemp()
{
	string T = "T";
	stringstream ss;
	ss << curtemp;
	curtemp++;		//临时变量个数加一
	T.append(ss.str());	//临时变量命名规则 T+当前个数
	return T;
}

//输出中间代码
void printmidcode(fstream &out)
{
	map<int, string>::iterator iter_i_s;
	for (iter_i_s = midcodemap.begin();iter_i_s != midcodemap.end();iter_i_s++)
		out << "(" << iter_i_s->first << ") " << iter_i_s->second << endl;
}

bool fomula_1(treebit *temp)
{
	//1 S -> program
	return true;
}
bool fomula_2(treebit *temp)
{
	return true;
}
bool fomula_3(treebit *temp)
{
	return true;
}
bool fomula_4(treebit *temp)
{
	return true;
}
bool fomula_5(treebit *temp)
{
	return true;
}
bool fomula_6(treebit *temp)
{
	return true;
}
bool fomula_7(treebit *temp)
{
	return true;
}
bool fomula_8(treebit *temp)
{
	return true;
}
bool fomula_9(treebit *temp)
{
	return true;
}
bool fomula_10(treebit *temp)
{
	return true;
}
bool fomula_11(treebit *temp)
{
	return true;
}
bool fomula_12(treebit *temp)
{
	return true;
}
bool fomula_13(treebit *temp)
{
	//13 fun-declaration -> type-specifier IDF ( params ) 
	temp = temp->father;
	curnum++;
	midcodemap[curnum] = proc + temp->sons[2]->re;		//插入中间代码 定义函数
	return true;
}
bool fomula_14(treebit *temp)
{
	return true;
}
bool fomula_15(treebit *temp)
{
	return true;
}
bool fomula_16(treebit *temp)
{
	return true;
}
bool fomula_17(treebit *temp)
{
	return true;
}
bool fomula_18(treebit *temp)
{
	return true;
}
bool fomula_19(treebit *temp)
{
	return true;
}
bool fomula_20(treebit *temp)
{
	return true;
}
bool fomula_21(treebit *temp)
{
	return true;
}
bool fomula_22(treebit *temp)
{
	return true;
}
bool fomula_23(treebit *temp)
{
	return true;
}
bool fomula_24(treebit *temp)
{
	return true;
}
bool fomula_25(treebit *temp)
{
	return true;
}
bool fomula_26(treebit *temp)
{
	return true;
}
bool fomula_27(treebit *temp)
{
	return true;
}
bool fomula_28(treebit *temp)
{
	return true;
}
bool fomula_29(treebit *temp)
{
	return true;
}
bool fomula_30(treebit *temp)
{
	return true;
}
bool fomula_31(treebit *temp)
{
	return true;
}
bool fomula_32(treebit *temp)
{
	return true;
}
bool fomula_33(treebit *temp)
{
	return true;
}
bool fomula_34(treebit *temp)
{
	return true;
}
bool fomula_35(treebit *temp)
{
	return true;
}
bool fomula_36(treebit *temp)
{
	return true;
}
bool fomula_37(treebit *temp)
{
	return true;
}
bool fomula_38(treebit *temp)
{
	return true;
}
bool fomula_39_1(treebit *temp)
{
	//39 selection-stmt -> if ( expression $39_1 ) statement $39_2 selection-stmt' $39_3
	temp = temp->father;
	temp->info->ifnum1 = curnum + 2;	//if语句真出口为当前curnum+2的位置
	curnum++;
	stringstream ss;
	ss << curnum + 2;
	//为真是强制跳转当前位置+2的位置
	midcodemap[curnum] = "if " + placelistmap[temp->sons[3]->nodenum][0] + "==1 Goto " + ss.str();
	curnum++;
	//为假强制跳转至假出口 暂时没有求出 
	midcodemap[curnum] = "Goto ";
	return true;
}
bool fomula_39_2(treebit *temp)
{
	//39 selection-stmt -> if ( expression $39_1 ) statement $39_2 selection-stmt' $39_3
	temp = temp->father;
	curnum++;
	midcodemap[curnum] = "Goto ";		//if语句为真执行语句结束 强制跳转至if语句下一条 暂时未求出
	temp->info->ifnum2 = curnum;		//得到if假出口位置前一条
	return true;
}
bool fomula_39_3(treebit *temp)
{
	//39 selection-stmt -> if ( expression $39_1 ) statement $39_2 selection-stmt' $39_3
	temp = temp->father;
	//emptyflag为1即不存在else语句 删除最后一天强制跳转语句 并且将之前空着的判断条件为假时的跳转位置填上
	if (temp->sons[8]->info->emptyflag)
	{
		stringstream ss;
		ss << temp->info->ifnum2;
		midcodemap[temp->info->ifnum1].append(ss.str());
		midcodemap.erase(curnum);
		curnum--;
	}	
	//emptyflag为0存在else语句 填充判断条件为假的跳转地址以及为真代码块结束的强制跳转地址
	else
	{
		stringstream ss;
		ss << temp->sons[8]->info->elsenum1 + 1;
		midcodemap[temp->info->ifnum1].append(ss.str());
		stringstream s;
		s << temp->sons[8]->info->elsenum2 + 1;
		midcodemap[temp->info->ifnum2].append(s.str());
	}		
	return true;
}
bool fomula_40(treebit *temp)
{
	//40 selection-stmt' -> empty 
	temp = temp->father;
	temp->info->emptyflag = 1;		//无else语句将emptyflag赋为1
	return true;
}
bool fomula_41_1(treebit *temp)
{
	//41 selection-stmt' -> else $41_1 statement $41_2
	temp = temp->father;
	temp->info->emptyflag = 0;
	temp->info->elsenum1 = curnum;		//记录else的入口地址 在43_3时填入
	return true;
}
bool fomula_41_2(treebit *temp)
{
	//41 selection-stmt' -> else $41_1 statement $41_2
	temp = temp->father;
	temp->info->emptyflag = 0;
	temp->info->elsenum2 = curnum;		//记录else的出口地址 在43_3时填入
	return true;
}
bool fomula_42_1(treebit *temp)
{
	//42 iteration-stmt -> while ( $42_1 expression $42_2 ) statement $42_3
	temp = temp->father;
	temp->info->whilenum1 = curnum + 1;		//记录while入口地址 42_3时跳转回此地址
	return true;
}
bool fomula_42_2(treebit *temp)
{
	//42 iteration-stmt -> while ( $42_1 expression $42_2 ) statement $42_3
	temp = temp->father;
	temp->info->whilenum2 = curnum + 1;
	curnum++;
	stringstream ss;
	ss << curnum + 2;
	//插入中间代码判断expression是否为真 为真强制跳转至当前地址+2 
	midcodemap[curnum] = "if " + placelistmap[temp->sons[4]->nodenum][0] + "==1 Goto " + ss.str();
	curnum++;
	//为假跳转至假出口 暂时未求出
	midcodemap[curnum] = "Goto ";
	return true;
}
bool fomula_42_3(treebit *temp)
{
	//42 iteration-stmt -> while ( $42_1 expression $42_2 ) statement $42_3
	temp = temp->father;
	stringstream ss;
	ss << curnum + 2;
	//回填假出口值
	midcodemap[temp->info->whilenum2 + 1].append(ss.str());
	curnum++;
	stringstream s;
	s << temp->info->whilenum1;
	//插入中间代码 强制跳转回42_1位置
	midcodemap[curnum]  = "Goto " + s.str();
	return true;
}
bool fomula_43(treebit *temp)
{
	//43 return-stmt -> return return-stmt'
	temp = temp->father;
	temp->info->returnflag = 1;
	if (temp->sons[2]->info->emptyflag)
	{
		curnum++;
		midcodemap[curnum] = "return";
	}	//无返回值
	else
	{
		curnum++;
		midcodemap[curnum] = "return " + placelistmap[temp->sons[2]->nodenum][0];
	}	//有返回值
	return true;
}
bool fomula_44(treebit *temp)
{
	//44 return-stmt' -> ; 
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_45(treebit *temp)
{
	//45 return-stmt' -> expression ; 
	temp = temp->father;
	temp->info->emptyflag = 0;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	return true;
}
bool fomula_46(treebit *temp)
{
	//46 expression -> var1 = expression 
	temp = temp->father;
	curnum++;
	// split
	string exp = placelistmap[temp->sons[3]->nodenum][0];
	string part1, part2;
	int split;
	split = exp.find("+");
	if(split != exp.npos){ // 如果expression中存在"+"
		part1.assign(exp, 0, split);
		part2.assign(exp, split+2, exp.length() - split - 2); 
		if(!strcmp(temp->sons[3]->varType, "char")){ // 是字符相加
			midcodemap[curnum++] = placelistmap[temp->sons[1]->nodenum][0] + "[0] := " + part1;
			midcodemap[curnum] = placelistmap[temp->sons[1]->nodenum][0] + "[1] := " + part2;
			return true;
		}
	}
	midcodemap[curnum] = placelistmap[temp->sons[1]->nodenum][0] + " := " + placelistmap[temp->sons[3]->nodenum][0];
	return true;
}
bool fomula_47(treebit *temp)
{
	//47 expression -> simple-expression
	temp = temp->father;
	strcpy_s(temp->varType, temp->sons[1]->varType);
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	return true;
}
bool fomula_48(treebit *temp)
{
	//48 var -> ID var'
	temp = temp->father;
	//
	//printf("temp->sons[1]->re: %s\n", temp->sons[1]->re);
	strcpy(temp->varType, getVarType(temp->sons[1]->re).c_str());
	//printf("temp->varType: %s\n", temp->varType);
	string ts;
	if (temp->sons[2]->info->arrayflag)
	{
		ts = temp->sons[1]->re;
		ts.append(placelistmap[temp->sons[2]->nodenum][0].c_str());
		placelistmap[temp->nodenum].push_back(ts);
	}
	else
		placelistmap[temp->nodenum].push_back(temp->sons[1]->re);
	return true;
}
bool fomula_49(treebit *temp)
{
	//49 var' -> empty 
	temp = temp->father;
	temp->info->arrayflag = 0;
	return true;
}
bool fomula_50(treebit *temp)
{
	//50 var' -> [ expression ]
	temp = temp->father;
	temp->info->arrayflag = 1;
	placelistmap[temp->nodenum].push_back("[" + placelistmap[temp->sons[2]->nodenum][0] + "]");
	//temp->info.placelist.push_back("[" + temp->sons[2]->info.placelist[0] + "]");
	return true;
}
bool fomula_51(treebit *temp)
{
	//51 var1 -> ID1 var1'
	temp = temp->father;
	string ts;
	if (temp->sons[2]->info->arrayflag)
	{
		ts = temp->sons[1]->re;
		ts.append(placelistmap[temp->sons[2]->nodenum][0]);
		placelistmap[temp->nodenum].push_back(ts);
	}
	else
		placelistmap[temp->nodenum].push_back(temp->sons[1]->re);
	return true;
}
bool fomula_52(treebit *temp)
{
	//52 var1' -> empty 
	temp = temp->father;
	temp->info->arrayflag = 0;
	return true;
}
bool fomula_53(treebit *temp)
{
	//53 var1' -> [ expression ] 
	temp = temp->father;
	temp->info->arrayflag = 1;
	placelistmap[temp->nodenum].push_back("[" + placelistmap[temp->sons[2]->nodenum][0] + "]");
	return true;
}
bool fomula_54(treebit *temp)
{
	//54 simple-expression -> additive-expression simple-expression'
	temp = temp->father;
	strcpy_s(temp->varType, temp->sons[1]->varType);
	if (temp->sons[2]->info->emptyflag)
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	else
	{
		string t = newtemp();
		placelistmap[temp->nodenum].push_back(t);
		curnum++;
		midcodemap[curnum] = t + " := " + placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0];
	}
	return true;
}
bool fomula_55(treebit *temp)
{
	//55 simple-expression' -> relop additive-expression 
	temp = temp->father;
	temp->info->emptyflag = 0;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0]);
	return true;
}
bool fomula_56(treebit *temp)
{
	//56 simple-expression' -> empty 
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_57(treebit *temp)
{
	//relop <=
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("<=");
	return true;
}
bool fomula_58(treebit *temp)
{
	//relop <
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("<");
	return true;
}
bool fomula_59(treebit *temp)
{
	//relop >
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(">");
	return true;
}
bool fomula_60(treebit *temp)
{
	//relop >=
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(">=");
	return true;
}
bool fomula_61(treebit *temp)
{
	//rolop ==
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("==");
	return true;
}
bool fomula_62(treebit *temp)
{
	//relop !=
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("!=");
	return true;
}
bool fomula_63(treebit *temp)
{
	//63 additive-expression -> term additive-expression'
	temp = temp->father;
	strcpy_s(temp->varType, temp->sons[1]->varType);
	//cout<<placelistmap[temp->sons[1]->nodenum][0]<<endl;
	if (temp->sons[2]->info->emptyflag)
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	else if(!strcmp(temp->varType, "char"))
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + placelistmap[temp->sons[2]->nodenum][0]);
	else
	{
		string t = newtemp();
		placelistmap[temp->nodenum].push_back(t);
		curnum++;
		midcodemap[curnum] = t + " := " + placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0];
	}
	return true;
}
bool fomula_64(treebit *temp)
{
	//64 additive-expression' -> addop term additive-expression' 
	temp = temp->father;
	temp->info->emptyflag = 0;
	if (temp->sons[3]->info->emptyflag)
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0]);
	else
	{
		string t = newtemp();
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + " " + t);
		curnum++;
		midcodemap[curnum] = t + " := " + placelistmap[temp->sons[2]->nodenum][0] + " " + placelistmap[temp->sons[3]->nodenum][0];
	}
	return true;
}
bool fomula_65(treebit *temp)
{
	//65 additive-expression' -> empty
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_66(treebit *temp)
{
	//addop +
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("+");             
	return true;
}
bool fomula_67(treebit *temp)
{
	//addop -
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("-");
	return true;
}
bool fomula_68(treebit *temp)
{
	//68 term -> factor term'
	temp = temp->father;
	strcpy_s(temp->varType, temp->sons[1]->varType);
	if (temp->sons[2]->info->emptyflag)
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	else
	{
		string t = newtemp();
		placelistmap[temp->nodenum].push_back(t);
		curnum++;
		midcodemap[curnum] = t + " := " + placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0];
	}
	return true;
}
bool fomula_69(treebit *temp)
{
	//69 term' -> mulop factor term' 
	temp = temp->father;
	temp->info->emptyflag = 0;
	if (temp->sons[3]->info->emptyflag)
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + " " + placelistmap[temp->sons[2]->nodenum][0]);
	else
	{
		string t=newtemp();
		placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0] + " " + t);
		curnum++;
		midcodemap[curnum] = t + " := " + placelistmap[temp->sons[2]->nodenum][0] + " " + placelistmap[temp->sons[3]->nodenum][0];
	}
	return true;
}
bool fomula_70(treebit *temp)
{
	//70 term' -> empty
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_71(treebit *temp)
{
	//mulop *
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("*");
	return true;
}
bool fomula_72(treebit *temp)
{
	//mulop /
	temp = temp->father;
	placelistmap[temp->nodenum].push_back("/");
	return true;
}
bool fomula_73(treebit *temp)
{
	//73 factor -> ( expression )
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[2]->nodenum][0]);
	return true;
}
bool fomula_74(treebit *temp)
{
	//74 factor -> var 
	temp = temp->father;
	strcpy_s(temp->varType, temp->sons[1]->varType); 
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	return true;
}
bool fomula_75(treebit *temp)
{
	//75 factor -> call
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	return true;
}
bool fomula_76(treebit *temp)
{
	//76 factor -> NUM
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(temp->sons[1]->re);
	return true;
}
bool fomula_77(treebit *temp)
{
	//77 call -> IDF ( args )
	temp = temp->father;
	string t;
	t = newtemp();
	placelistmap[temp->nodenum].push_back(t);
	if (!temp->sons[3]->info->emptyflag)
		for (iter = placelistmap[temp->sons[3]->nodenum].begin();iter != placelistmap[temp->sons[3]->nodenum].end();iter++)
		{
			curnum++;
			midcodemap[curnum] = param + *iter;
		}
	curnum++;
	midcodemap[curnum] = t + " := " + call + temp->sons[1]->re;
	return true;
}
bool fomula_78(treebit *temp)
{
	//78 args -> arg-list 
	temp = temp->father;
	temp->info->emptyflag = 0;
	for (iter = placelistmap[temp->sons[1]->nodenum].begin();iter != placelistmap[temp->sons[1]->nodenum].end();iter++)
		placelistmap[temp->nodenum].push_back(*iter);
	return true;
}
bool fomula_79(treebit *temp)
{
	//79 args -> empty
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_80(treebit *temp)
{
	//80 arg-list -> expression arg-list'
	temp = temp->father;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[1]->nodenum][0]);
	if (!temp->sons[2]->info->emptyflag)
		for (iter = placelistmap[temp->sons[2]->nodenum].begin();iter != placelistmap[temp->sons[2]->nodenum].end();iter++)
			placelistmap[temp->nodenum].push_back(*iter);
	return true;
}
bool fomula_81(treebit *temp)
{
	//81 arg-list' -> , expression arg-list' 
	temp = temp->father;
	temp->info->emptyflag = 0;
	placelistmap[temp->nodenum].push_back(placelistmap[temp->sons[2]->nodenum][0]);
	if (!temp->sons[3]->info->emptyflag)
		for (iter = placelistmap[temp->sons[3]->nodenum].begin();iter != placelistmap[temp->sons[3]->nodenum].end();iter++)
			placelistmap[temp->nodenum].push_back(*iter);
	return true;
}
bool fomula_82(treebit *temp)
{
	//82 arg-list' -> empty
	temp = temp->father;
	temp->info->emptyflag = 1;
	return true;
}
bool fomula_83(treebit* temp){
	return true;
}

//初始化语义动作表
void init_funcmap()
{
	funcmap["$1"] = fomula_1;
	funcmap["$2"] = fomula_2;
	funcmap["$3"] = fomula_3;
	funcmap["$4"] = fomula_4;
	funcmap["$5"] = fomula_5;
	funcmap["$6"] = fomula_6;
	funcmap["$7"] = fomula_7;
	funcmap["$8"] = fomula_8;
	funcmap["$9"] = fomula_9;
	funcmap["$10"] = fomula_10;
	funcmap["$11"] = fomula_11;
	funcmap["$12"] = fomula_12;
	funcmap["$13"] = fomula_13;
	funcmap["$14"] = fomula_14;
	funcmap["$15"] = fomula_15;
	funcmap["$16"] = fomula_16;
	funcmap["$17"] = fomula_17;
	funcmap["$18"] = fomula_18;
	funcmap["$19"] = fomula_19;
	funcmap["$20"] = fomula_20;
	funcmap["$21"] = fomula_21;
	funcmap["$22"] = fomula_22;
	funcmap["$23"] = fomula_23;
	funcmap["$24"] = fomula_24;
	funcmap["$25"] = fomula_25;
	funcmap["$26"] = fomula_26;
	funcmap["$27"] = fomula_27;
	funcmap["$28"] = fomula_28;
	funcmap["$29"] = fomula_29;
	funcmap["$30"] = fomula_30;
	funcmap["$31"] = fomula_31;
	funcmap["$32"] = fomula_32;
	funcmap["$33"] = fomula_33;
	funcmap["$34"] = fomula_34;
	funcmap["$35"] = fomula_35;
	funcmap["$36"] = fomula_36;
	funcmap["$37"] = fomula_37;
	funcmap["$38"] = fomula_38;
	funcmap["$39_1"] = fomula_39_1;
	funcmap["$39_2"] = fomula_39_2;
	funcmap["$39_3"] = fomula_39_3;
	funcmap["$40"] = fomula_40;
	funcmap["$41_1"] = fomula_41_1;
	funcmap["$41_2"] = fomula_41_2;
	funcmap["$42_1"] = fomula_42_1;
	funcmap["$42_2"] = fomula_42_2;
	funcmap["$42_3"] = fomula_42_3;
	funcmap["$43"] = fomula_43;
	funcmap["$44"] = fomula_44;
	funcmap["$45"] = fomula_45;
	funcmap["$46"] = fomula_46;
	funcmap["$47"] = fomula_47;
	funcmap["$48"] = fomula_48;
	funcmap["$49"] = fomula_49;
	funcmap["$50"] = fomula_50;
	funcmap["$51"] = fomula_51;
	funcmap["$52"] = fomula_52;
	funcmap["$53"] = fomula_53;
	funcmap["$54"] = fomula_54;
	funcmap["$55"] = fomula_55;
	funcmap["$56"] = fomula_56;
	funcmap["$57"] = fomula_57;
	funcmap["$58"] = fomula_58;
	funcmap["$59"] = fomula_59;
	funcmap["$60"] = fomula_60;
	funcmap["$61"] = fomula_61;
	funcmap["$62"] = fomula_62;
	funcmap["$63"] = fomula_63;
	funcmap["$64"] = fomula_64;
	funcmap["$65"] = fomula_65;
	funcmap["$66"] = fomula_66;
	funcmap["$67"] = fomula_67;
	funcmap["$68"] = fomula_68;
	funcmap["$69"] = fomula_69;
	funcmap["$70"] = fomula_70;
	funcmap["$71"] = fomula_71;
	funcmap["$72"] = fomula_72;
	funcmap["$73"] = fomula_73;
	funcmap["$74"] = fomula_74;
	funcmap["$75"] = fomula_75;
	funcmap["$76"] = fomula_76;
	funcmap["$77"] = fomula_77;
	funcmap["$78"] = fomula_78;
	funcmap["$79"] = fomula_79;
	funcmap["$80"] = fomula_80;
	funcmap["$81"] = fomula_81;
	funcmap["$82"] = fomula_82;
	funcmap["$83"] = fomula_83;
}

//使用深度优先遍历访问语法分析树
void deepsearch(treebit *temp,fstream &out)
{
	if (strstr(temp->type, "derivation") != 0)
	{
		//out << temp->re << " " << temp->nodenum << endl;
		for (int i = 1;i <= temp->bitnum;i++)
			deepsearch(temp->sons[i],out);
	}
	else
	{
		//out << temp->re << endl;
		stringstream ss;
		ss << temp->re;
		if (ss.str().find("$") != ss.str().npos)
			funcmap[ss.str()](temp);
	}
}

//生成中间代码
void midcode(fstream &in, fstream &out)
{
	treebit *temp;
	init_funcmap();
	temp = treenode;
	deepsearch(temp->sons[2],out);
	printmidcode(out);
}
