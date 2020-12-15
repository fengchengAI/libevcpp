#include <string>
#include <iostream>
#include <vector>

using namespace std;
class A{
	public:
	int * ptr;
};

class B{
	public:
	string str;
};
template <typename T>
class Base
{
public:

vector<A*>data;
};

int main(){
	Base<A> *a = new Base<A>();
	a->data.push_back(new A());
 
	Base<B> *b = new Base<B>();
}
