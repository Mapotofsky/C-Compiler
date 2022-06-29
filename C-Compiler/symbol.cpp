#include "symbol.h"

int funcnum = 0;				//���ű���Ԫ�ظ���

								//�жϺ����Ƿ��Ѿ����� �������ڸú����ں��������е�λ�� �������򷵻�-1
int func_exist(struct func functable[255], int funcnum, string str)
{
	int i;
	for (i = 0;i < funcnum;i++)
		if (functable[i].name == str)
			return i;
	return -1;
}

//�жϱ����Ƿ�����ڵ�ǰ�����ķ��ű��� �����򷵻ر���λ�� ���򷵻�-1
int sym_exist(struct func functable[255], struct symbol *st, int num, string str)
{
	for (int i = 0; i < functable[num].num; i++)
		if ((st + i)->name == str)
			return i;
	return -1;
}

// ��õ�ǰ����������
string getType(string var, int num){
	for(int i=0; i<functable[num].num; i++) // ������ǰ�������ű��ҵ�����������
		if((functable[num].st+i)->name == var)
			return (functable[num].st+i)->type;
}

// ��õ�ǰ����������
string getProperty(string var, int num){
	for(int i=0; i<functable[num].num; i++) // ������ǰ�������ű��ҵ�����������
		if((functable[num].st+i)->name == var)
			return (functable[num].st+i)->property;
}

//����ǰ���������뵽��ǰ��������
void pushsym(struct func functable[255], struct symbol *st, int curnum, int num) // curnum:��ǰ�������
{
	st[functable[curnum].num].name = functable[num].name;
	st[functable[curnum].num].property = "IDF";
	st[functable[curnum].num].type = "-";
	st[functable[curnum].num].st = functable[num].st;
	functable[curnum].num++;
}

//����ǰ�������뵽��ǰ��������
void pushsym2(struct func functable[255], struct symbol *st, int curnum, int num, string name, string property, string type)
{
	st[functable[curnum].num].name = name;
	st[functable[curnum].num].property = property;
	st[functable[curnum].num].type = type;
	functable[curnum].num++;
}

//����ǰ�������뵽�������� �����ظú������ű�ĵ�ַ
struct symbol * pushfunc(struct func functable[255], int &funcnum, string str, string ret)
{
	functable[funcnum].name = str;
	functable[funcnum].num = 0;
	functable[funcnum].ret = ret;
	funcnum++;
	return functable[funcnum - 1].st;
}

//��ʼ�����ű�
void init_symtable(void)
{
	pushfunc(functable, funcnum, "input", "-");
	pushfunc(functable, funcnum, "output", "-");
}

//������ű�
void output(fstream &out)
{
	int i, j;
	int set = 20;
	//����������ű�
	out << setw(set) << left << "������" << setw(set) << "�����в�������" << setw(set) << "������������" << setw(set) << "�������ű��ַ" << endl;
	for (i = 0;i < funcnum;i++)
		out << setw(set - 1) << left << functable[i].name << setw(set - 2) << functable[i].num << setw(set - 2) << functable[i].ret << "0x" << setw(set) << functable[i].st << endl;
	//�䴦��������
	for (i = 0;i < funcnum;i++)
	{
		out << endl;
		out << "������:" << functable[i].name << "\t���ű��ַ:0x" << functable[i].st << endl;
		if (functable[i].num)
			out << setw(set) << "������" << setw(set) << "����" << setw(set) << "����" << setw(set) << "��ַ" << endl;
		for (j = 0;j < functable[i].num;j++)
			out << setw(set - 1) << functable[i].st[j].name << setw(set - 1) << functable[i].st[j].property << setw(set - 1) << functable[i].st[j].type << setw(set) << functable[i].st[j].st << endl;
	}
}

//���ɷ��ű�
void calsymboltable(fstream &in, fstream &out)
{
	vector<Token> tokens;
	Token token;
	struct symbol *cur = NULL;
	int flag1, flag2, num, curnum;
	//����ʷ��������
	while (!in.eof())
	{
		in >> token.re >> token.type >> token.line >> token.colume;
		if (in.fail())
			break;
		tokens.push_back(token);
	}
	flag1 = 0; // �����ű��
	flag2 = 0; // С���ű��
	init_symtable();


	//��⵱ǰtoken������Ϊ���������߱����������ݱ�����ŵ�flag1.flag2�ж�λ�� 
	for (vector<Token>::iterator titer = tokens.begin();titer != tokens.end();titer++)
	{
		if (titer->type == "IDF")	//��ǰtokenΪ������,��Ϊ�����Ķ���͵������ 
		{
			num = func_exist(functable, funcnum, titer->re);
			if (flag1 == 0)//֮ǰ������������ţ�˵����ǰΪ�����Ķ���
			{
				if (num != -1)//�Ѵ��ڸú�����
				{
					cur = functable[num].st;  //�õ��ú������ڷ��ű��е���Ϣ
					curnum = num;
				}
				else     //�����ڸú�����,д��ú����� 
				{
					cur = pushfunc(functable, funcnum, titer->re, (titer - 1)->re);
					curnum = funcnum - 1;
				}
			}
			else if (flag1 > 0)   //˵���Ǻ����ڲ��Ժ����ĵ���
			{
				if (num == -1)	//�������ű���δ���� ����û�б�����ͱ����� ����
				{
					cout << "Undefined Function [" << titer->re << "] at Line " << titer->line << " Colume " << titer->colume << endl;
					exit(0);
				}
				else//�Ѵ��ڸú��������ж��Ƿ�Ϊ�״ε��� �����������ǰ�����ı�������
				{
					if (sym_exist(functable, cur, curnum, titer->re) == -1)
						pushsym(functable, cur, curnum, num);
				}
			}
		}

		else if (titer->type == "ID")											//��ǰtokenΪ����������Ϊ�����������͵������ 
		{
			if ((titer - 1)->re == "void" || (titer - 1)->re == "int" || (titer - 1)->re == "char")		//ǰһ��tokenΪint��void��˵����ǰΪ���������� 
			{
				if (flag1 == 0 || (flag1 > 0 && flag2 == 0)) 				//��������ֻ���ں�����ȡ�β��л�������λ�ں����ڲ���С������ 
				{
					if (sym_exist(functable, cur, curnum, titer->re) == -1) 	//�����ڸñ��������壬��� 
						if((titer+1)->re == "[") // ������������������ţ���˵�������鶨��
							pushsym2(functable, cur, curnum, num, titer->re, "ARRAY", (titer - 1)->re);
						else  // ��������
							pushsym2(functable, cur, curnum, num, titer->re, "VARIABLE", (titer - 1)->re);
					else														//���ڸñ������������ 
					{
						cout << "Variable [" << titer->re << "] has been defined!" << endl;
						exit(0);
					}
				}
				else
				{
					cout << "//���������ڴ����λ�� ";
				}
			}
			else															//ǰһ��token����int��void��˵��Ϊ�����ĵ��� 
			{
				if (sym_exist(functable, cur, curnum, titer->re) == -1)			//���ű��в����ڣ�˵��δ�����ʹ��
				{
					cout << "Undefined Variable [" << titer->re << "] at Line " << titer->line << " Colume " << titer->colume << endl;
					exit(0);
				}
			}
		}
		else if(titer->type == "OOP"){ // �����������OOP�����ж�ǰ������������Ƿ���ͬ���Ƿ��Ǹ������֧�ֵ�����
			// ����������token����
			string pre_type;
			if((titer-1)->re == "]"){ // ��ǰһ��token�������ţ���˵����������������е�Ԫ��
				vector<Token>::iterator it;
				// ��ǰ����token��ֱ���ҵ�"[",��ǰһ������Ŀ��token
				for(it = titer-2; it->re!="[" ;it--)
					continue;
				it--;
				// ����ű�õ�������
				pre_type = getType(it->re, curnum);
			}
			else
				pre_type= ((titer-1)->type == "NUM" ? "int" : getType((titer-1)->re, curnum));
			if(pre_type == "void" || (pre_type == "char" && titer->re != "+")){
				out << "Grammar Error at Line " << (titer-1)->line << " Colume " << (titer-1)->colume << endl;
				cout << "Grammar Error at Line " << (titer-1)->line << " Colume " << (titer-1)->colume << endl;
				exit(0);
			}
			// ����Ҳ�����token����
			string next_type = ((titer+1)->type == "NUM" ? "int" : getType((titer+1)->re, curnum));
			if(pre_type == "void" || (pre_type == "char" && titer->re != "+") || pre_type != next_type){
				out << "Grammar Error at Line " << (titer+1)->line << " Colume " << (titer+1)->colume << endl;
				cout << "Grammar Error at Line " << (titer+1)->line << " Colume " << (titer+1)->colume << endl;
				exit(0);
			}
		}
		// �����������ţ��ж��Ƿ�����������
		else if(titer->re == "["){
			string pre_property = getProperty((titer-1)->re, curnum);
			if(pre_property != "ARRAY"){ // ��������������ͣ�����
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

