#ifndef MYGRAPH_H
#define MYGRAPH_H

#include <iostream>
#include <vector>
#include <queue>
#include <stack>

using namespace std;

class Node{
public:	
	int dest;
	double weight;

	Node(int d, double w){
		dest = d;
		weight = w;
	}
};

class Graph{
public:
	vector<vector<Node> > W;
	vector<int> sgroup;
	vector<int> tgroup;
	double maxflow;

	void assignW(vector<vector<Node> > &WW);
	bool bfs(vector<int> &parent, vector<bool> &visited);
	bool dfs(vector<int> &parent, vector<bool> &visited);
	void mincut();
	void printgraph();
	vector<bool> getmincutresult();
};

#endif 