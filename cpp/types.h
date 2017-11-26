
struct VertexProperties {
	long int id;
	template <typename Ar> void serialize(Ar& ar, unsigned) { 
	ar & id; 
}
};

struct EdgeProperties {
	long int id;
	long int idx;
	long int source;
	long int target;
	double weight;
	template <typename Ar> void serialize(Ar& ar, unsigned) { 
	ar & id;
	ar & idx;
	ar & source;
	ar & target;
	ar & weight; 
}

};



struct VertexG {
	long int id;
	double x;
	double y;
};

struct EdgeG {
	long int id;
	long int idx;
	long int source;
	long int target;
	double weight;
	int level;
};

struct PromotedEdge {
	long int id;
	long int source;
	long int target;
	int level;
};

struct Connection {
	long int source;
	long int target;
	int level;
};
