#ifndef OUTPUT_H
#define OUTPUT_H

#include <gringo.h>
#include <value.h>
#include <ext/hash_map>

namespace NS_GRINGO
{
	namespace NS_OUTPUT
	{
		typedef std::vector<Object*> ObjectVector;

		class Output
		{
		protected:
			typedef __gnu_cxx::hash_map<ValueVector, std::pair<std::string, int>, Value::VectorHash> AtomHash;
			typedef std::vector<AtomHash> AtomLookUp;
		public:
			Output(std::ostream *out);
			virtual void initialize(Grounder *g);
			virtual void print(NS_OUTPUT::Object *o) = 0;
			virtual void finalize() = 0;
			bool addAtom(NS_OUTPUT::Atom *r);
			virtual int newUid();
			virtual ~Output();
		protected:
			int uids_;
			std::ostream *out_;
			Grounder *g_;
			AtomLookUp atoms_;
		};
	
		struct Object
		{
			Object(int type);
			Object(bool neg, int type);
			virtual void print(std::ostream &out) = 0;
			virtual void print_plain(std::ostream &out) = 0;
			virtual void addUid(Output *o) = 0;
			virtual void addDomain(bool fact = true) = 0;
			int getUid();
			virtual ~Object();
			
			bool neg_;
			const int type_;
			int uid_;
		};

		struct Atom : public Object
		{
			Atom(bool neg, Node *node, ValueVector &values);
			void addDomain(bool fact);
			void addUid(Output *o);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			
			bool visible_;
			bool print_;
			Node *node_;
			std::string *name_;
			ValueVector values_;
		};

		struct Rule : public Object
		{
			Rule(Object* head, Object *body);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Rule();
			void addDomain(bool fact);
			void addUid(Output *o);

			Object *head_;
			Object *body_;
		};

		struct Fact : public Object
		{
			Fact(Object *head);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Fact();
			void addDomain(bool fact);
			void addUid(Output *o);

			Object *head_;
		};
		
		struct Integrity : public Object
		{
			Integrity(Object *body);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			void addDomain(bool fact);
			void addUid(Output *o);
			~Integrity();

			Object *body_;
		};

		struct Conjunction : public Object
		{
			Conjunction(ObjectVector &lits);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Conjunction();
			void addDomain(bool fact);
			void addUid(Output *o);

			ObjectVector lits_;
		};

		struct Aggregate : public Object
		{
			enum Type   { SUM = 0xe, COUNT = 0xf, MAX = 0x10, MIN=0x11, TIMES=0x12 };
			enum Bounds { LU = 3, U = 2, L = 1, N = 0 };
			Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights, int upper);
			Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights);
			Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights, int upper);
			Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			void addDomain(bool fact);
			void addUid(Output *o);
			~Aggregate();

			ObjectVector lits_;
			IntVector    weights_;
			Bounds       bounds_;
			int          lower_;
			int          upper_;
		};
		
		struct Compute : public Object
		{
			Compute(ObjectVector &lits, int models);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Compute();
			void addDomain(bool fact);
			void addUid(Output *o);

			ObjectVector lits_;
			int models_;
		};	
		
		struct Optimize : public Object
		{
			enum Type { MINIMIZE, MAXIMIZE };
			Optimize(Type type, ObjectVector &lits, IntVector &weights);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Optimize();
			void addDomain(bool fact);
			void addUid(Output *o);

			ObjectVector lits_;
			IntVector    weights_;
		};	
	}
}

#endif

