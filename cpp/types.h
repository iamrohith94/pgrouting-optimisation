
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
	template <typename Ar> void serialize(Ar& ar, unsigned) { 
		ar & id; 
		ar & x; 
		ar & y; 
	}
};

struct EdgeG {
	long int id;
	long int idx;
	long int source;
	long int target;
	double weight;
	int level;

	template <typename Ar> void serialize(Ar& ar, unsigned) { 
	ar & id;
	ar & idx;
	ar & source;
	ar & target;
	ar & weight;
	ar & level; 
	}
};

struct PromotedEdge {
	long int id;
	long int source;
	long int target;
	int level;
	template <typename Ar> void serialize(Ar& ar, unsigned) { 
		ar & id; 
		ar & source; 
		ar & target;
		ar & level;
	}
};

struct Connection {
	long int source;
	long int target;
	int level;
};

struct Shortcut {
	long int source;
	long int target;
	double cost;
	double reverse_cost;
};
