#include "mygraph.hpp"

/******************************************************************************
 DESCRIPTION:
	Assign the edge matrix to the graph.

 ARGUMENT:
	WW: the edge matrix.

*******************************************************************************/
void Graph::assignW(vector<vector<Node> > &WW){
	W = WW;
	return;
}


/******************************************************************************
 DESCRIPTION:
	Perform bfs on graph and store the path from s to t in [parent].

 OUTPUT:
	parent: store which vertice is your parent in the path.

 RETURN:
 	Return true if there is a path from s to t, vice versa.
*******************************************************************************/
bool Graph::bfs(vector<int> &parent, vector<bool> &visited){
	
	int source = 0;
	int target = W.size()-1;

	// the visited array, all vertices are not visited yet 
	visited = vector<bool>((int)W.size(), false);

	// init bfs	
	queue<int> q;
	q.push(source);
	visited[source] = true;
	parent[source] = -1;

	// bfs loop
	while(!q.empty()){
		int u = q.front();
		q.pop();

		for(int i = 0; i < W[u].size(); i++){
			int v = W[u][i].dest;
			double w = W[u][i].weight;
			if(visited[v] == false && w > 0){
				q.push(v);
				parent[v] = u;
				visited[v] = true;
				if(v == target) return true;
			}
		}
	}
	return false;
}

/******************************************************************************
 DESCRIPTION:
	Perform dfs on graph and store the path from s to t in [parent].

 OUTPUT:
	parent: store which vertice is your parent in the path.

 RETURN:
 	Return true if there is a path from s to t, vice versa.
*******************************************************************************/
bool Graph::dfs(vector<int> &parent, vector<bool> &visited){
	
	int source = 0;
	int target = W.size()-1;
	
	// the visited array, all vertices are not visited yet 
	visited = vector<bool>((int)W.size(), false);
	
	// init dfs
	vector<int> hasrun((int)W.size(), 0);
	stack<int> s;
	s.push(source);
	visited[source] = true;
	parent[source] = -1;

	// dfs loop
	while(!s.empty()){
		int u = s.top();
		if(hasrun[u] == W[u].size()){
			s.pop();
			continue;
		}
		int uchild = hasrun[u];
		hasrun[u]++;

		int v = W[u][uchild].dest;
		double w = W[u][uchild].weight;
		if(visited[v] == false && w > 0){
			s.push(v);
			parent[v] = u;
			visited[v] = true;
			if(v == target) return true;
		}
	}
	return false;
}

/******************************************************************************
 DESCRIPTION:
	Ford-Fulkerson mincut algorithm.

*******************************************************************************/
void Graph::mincut(){
	cerr << "INFO/GRAPH.MINCUT:" << endl;
	int source = 0;
	int target = W.size()-1;

	vector<int> parent((int)W.size(), 0);
	vector<bool> visited((int)W.size(), false);

	while(bfs(parent, visited)){
		// find minimum residual capacity according to the path
		double pathflow = 100000;
		for(int v = target; v != source; v = parent[v]){
			int u = parent[v];
			for(int i = 0; i < W[u].size(); i++){
				if(W[u][i].dest == v){
					pathflow = min(pathflow, W[u][i].weight);
					break;
				}
			}
		}
		
		// update residual capacity of the edge along the path
		for(int v = target; v != source; v = parent[v]){
			int u = parent[v];
			for(int i = 0; i < W[u].size(); i++){
				if(W[u][i].dest == v){
					W[u][i].weight -= pathflow;
					break;
				}
			}
			bool edgeexist = false; 
			for(int i = 0; i < W[v].size(); i++){
				if(W[v][i].dest == u){
					W[v][i].weight += pathflow;
					edgeexist = true;
					break;
				}
			}
			if(!edgeexist){
				Node n(u, pathflow);
				W[v].push_back(n);
			}
		}
	}

	// update the segment result
	sgroup.clear();
	tgroup.clear();
	for(int i = 0; i < visited.size(); i++){
		if(visited[i]) sgroup.push_back(i);
		else tgroup.push_back(i);
	}
	cerr << "	sgroup # = " << (int)sgroup.size() << endl;
	cerr << "	tgroup # = " << (int)tgroup.size() << endl;

	// count maxflow
	maxflow = 0;
	for(int i = 0; i < W[target].size(); i++){
		maxflow += W[target][i].weight;
	}
	cerr << "	done." << endl;
	return;
}

void Graph::printgraph(){
	cerr << "INFO/GRAPH.PRINTGRAPH:" << endl;
	for(int i = 0; i < W.size(); i++){
		for(int j = 0; j < W[i].size(); j++){
			cerr << "	(" << i << ", " << W[i][j].dest << ") = " << W[i][j].weight << endl;
		}
	}
}

vector<bool> Graph::getmincutresult(){
	vector<bool> result(W.size(), 0);
	for(int i = 0; i < sgroup.size(); i++){
		result[sgroup[i]] = 1;
	}
	return result;
}



