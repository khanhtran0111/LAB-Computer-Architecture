#include <string.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <tuple>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <stack>
#include <queue>
#include <tuple>
#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <omp.h>
#include <chrono>

#include "CycleTimer.h"

#define OUTGOING 0
#define INCOMING 1

using namespace std;

template <typename T>
class VQueue
{
	size_t V;
	size_t curr_;
	size_t next_;
	size_t end_;

public:
	vector<T> data;
	vector<T> value;
	explicit VQueue(size_t V) : data(V), value(V), V(V), curr_(0), next_(0), end_(0)
	{
	}
	inline bool empty() const { return curr_ == next_; }
	inline bool full() const { return end_ == V; }
	inline T &front() { return data[curr_]; }
	inline size_t size() const { return end_; }
	inline void pop()
	{
		++curr_;
		assert(curr_ <= end_);
	}
	inline void push(const T &val)
	{
		data[end_++] = val;
		assert(end_ <= V);
	}
	inline void push(const T &node, const T &val)
	{
		data[end_] = node;
		value[end_++] = val;
		assert(end_ <= V);
	}
	inline void next()
	{
		assert(curr_ == next_);
		next_ = end_;
	} 
	inline void clear() { curr_ = next_ = end_ = 0; }
	inline void resize(size_t V_)
	{
		if (V_ > V)
		{
			V = V_;
			data.resize(V);
			value.resize(V);
		}
	}
	inline size_t sum()
	{
		size_t s = 0;
		for (uint32_t i = 0; i < end_; i++)
		{
			s += value[i];
		}
		return s;
	}

	inline typename vector<T>::iterator begin() { return data.begin(); }
	inline typename vector<T>::iterator end() { return data.begin() + end_; }
};

const uint32_t ALIVE = 0;
const uint32_t DEAD = 1;
const uint32_t UNKNOWN = 2;
const uint32_t MASK = 3;

vector<tuple<uint32_t, uint32_t, uint32_t, char>> updates; // <source, target, timestamp, type>
vector<tuple<uint32_t, uint32_t, uint32_t>> queries;	   // <source, target, timestamp>
vector<vector<uint32_t>> Edges;
vector<vector<uint32_t>> Maps;	 // for all threads
vector<VQueue<uint32_t>> Queues; // for all threads
vector<int> Distances;			 // for all nodes
uint32_t nodeNum = 0, edgeNum = 0;
bool diGraph = false, verbose = false;
uint32_t testNum = 1;
uint32_t threadNum = 8;

static inline uint32_t GetID(uint32_t v) { return v >> 2; }
static inline uint32_t GetState(uint32_t v) { return v & MASK; }
static inline uint32_t ToEdge(uint32_t v) { return (v << 2); } //| ALIVE
static inline void ResetMap(uint32_t threadID)
{
	for (uint32_t v : Queues[threadID].data)
	{
		Maps[threadID][v >> 5] = 0;
	}
}
// set bit at v in Maps
static inline void SetBit(uint32_t threadID, uint32_t v)
{
	Maps[threadID][v >> 5] |= (1 << (v & 31));
}

// test bit at v in Maps
static inline int TestBit(uint32_t threadID, uint32_t v)
{
	return (Maps[threadID][v >> 5] >> (v & 31)) & 1;
}

inline bool readeof()
{
	for (;;)
	{
		int c = getc(stdin);
		if (c == EOF || c == '#')
		{
			return true;
		}
		else if (isspace(c))
		{
			continue;
		}
		else
		{
			ungetc(c, stdin);
			return false;
		}
	}
	assert(false);
}

inline uint32_t readuint()
{
	uint32_t c;
	uint32_t x;
	while (!isdigit(c = getc(stdin)))
		;
	x = c - '0';
	while (isdigit(c = getc(stdin)))
	{
		x = (x * 10 + (c - '0'));
	}
	return x;
}

void Build(char *filename)
{
    cerr << "Building the big graph...";

    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        cerr << "Error: Cannot open file " << filename << endl;
        exit(1);
    }

    const size_t MAX_LINE_LENGTH = 1024;
    char line[MAX_LINE_LENGTH];
    uint32_t u, v;

    while (fgets(line, MAX_LINE_LENGTH, fp))
    {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == 'S')
            break;

        if (sscanf(line, "%u %u", &u, &v) != 2)
            continue;

        nodeNum = max({nodeNum, u + 1, v + 1});
        if (nodeNum > Edges.size())
        {
            Edges.resize(nodeNum + 1e6);
        }
        Edges[u].push_back(v);
        if (!diGraph)
            Edges[v].push_back(u);
    }

    fclose(fp);
    cerr << " Done." << endl;

    // Reduce Edges buffer to current nodeNum
    Edges.resize(nodeNum);
    edgeNum = 0;

    // Sort adjacent lists
    for (uint32_t v = 0; v < nodeNum; v++)
    {
        sort(Edges[v].begin(), Edges[v].end());
        Edges[v].erase(unique(Edges[v].begin(), Edges[v].end()), Edges[v].end());
        edgeNum += Edges[v].size();
    }

    // Initialize the graph
    Queues.clear();
    Maps.clear();
    for (uint32_t t = 0; t < threadNum; t++)
    {
        Queues.emplace_back(VQueue<uint32_t>(nodeNum));
        Maps.emplace_back(vector<uint32_t>(nodeNum / 32 + 1));
    }
    Distances.resize(nodeNum);

    cerr << "Statistics: This graph has " << nodeNum << " vertices and " << edgeNum << " edges." << endl;
}


// Compute in serial the Closeness Centrality for all nodes
void computeCCserial()
{
	// vector<tuple<uint32_t, int>> Distances(nodeNum);
	vector<double> CC(nodeNum);

	for (uint32_t v = 0; v < nodeNum; v++)
	{
		uint32_t threadID = 0;
		// travel SSSP from v
		auto &Q = Queues[threadID];
		uint32_t distance = 0;
		Q.clear();
		Q.push(v, 0);
		Q.next();
		SetBit(threadID, v);
		while (!Q.empty())
		{
			distance++;
			while (!Q.empty())
			{
				uint32_t s = Q.front();
				Q.pop();
				for (uint32_t w : Edges[s])
				{
					if (TestBit(threadID, w))
					{
						continue;
					}
					Q.push(w, distance);
					SetBit(threadID, w);
				}
			}
			Q.next();
		}
		ResetMap(threadID);
		distance = Q.sum();
		if (distance == 0)
			CC[v] = 0.0;
		else
			CC[v] = ((double)(nodeNum - 1) / distance);
	}

	// show results
	if (verbose)
	{
		for (uint32_t v = 0; v < nodeNum; v++)
		{
			cerr << v << "==" << std::setprecision(4) << CC[v] << endl;
		}
	}
}

// Compute in parallel the Closeness Centrality for all nodes
void computeCCparallel()
{
    auto start = chrono::high_resolution_clock::now();
    
    vector<double> CC(nodeNum, 0.0);
    
    #pragma omp parallel for
    for (uint32_t v = 0; v < nodeNum; v++)
    {
        uint32_t threadID = omp_get_thread_num();  
        auto &Q = Queues[threadID]; 
        uint32_t distance = 0;
        
        Q.clear();
        Q.push(v, 0);
        Q.next();
        
        SetBit(threadID, v);
        
        while (!Q.empty())
        {
            distance++;
            while (!Q.empty())
            {
                uint32_t s = Q.front();
                Q.pop();
                for (uint32_t w : Edges[s])
                {
                    if (TestBit(threadID, w))
                    {
                        continue;
                    }
                    Q.push(w, distance);
                    SetBit(threadID, w);
                }
            }
            Q.next();
        }
        
        ResetMap(threadID);
        distance = Q.sum();
        
        if (distance == 0)
        {
            CC[v] = 0.0;
        }
        else
        {
            CC[v] = (double)(nodeNum - 1) / distance;
        }
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Time: " << duration.count() << " milliseconds" << endl;

    if (verbose)
    {
        for (uint32_t v = 0; v < nodeNum; v++)
        {
            cerr << v << "==" << std::setprecision(4) << CC[v] << endl;
        }
    }
}


// main function
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		cout << "Check params! Syntax is 'bigGraph graph-filename is-directed(0-1) method(CC-SCC) is-verbose(0-1)'" << endl;
		return 0;
	}
	cout << "Maximum threads in the computer = " << threadNum << endl;
	diGraph = atoi(argv[2]);
	
	Build(argv[1]);
	auto method = std::string(argv[3]);
	if (argc == 5)
		verbose = atoi(argv[4]);
	auto ccStart = chrono::high_resolution_clock::now();
	if (method == "CC")
	{
		cout << "Calculating in parallel the Closeness Centrality of all nodes in graph ..." << endl;
		computeCCparallel();
	}
	else if (method == "SCC")
	{
		cout << "Calculating in serial the Closeness Centrality of all nodes in graph ..." << endl;
		computeCCserial();
	}
	auto ccEnd = chrono::high_resolution_clock::now();
	auto ccDuration = chrono::duration_cast<std::chrono::milliseconds>(ccEnd - ccStart);
	cout << "Closeness Centrality calculation time: " << ccDuration.count() << " milliseconds" << endl;
	cout << "Done!" << endl;
	return 0;
}