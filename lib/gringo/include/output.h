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
			typedef __gnu_cxx::hash_map<ValueVector, int, Value::VectorHash, Value::VectorEqual> AtomHash;
			typedef std::vector<AtomHash> AtomLookUp;
		public:
			Output(std::ostream *out);
			virtual void initialize(SignatureVector *pred);
			virtual void print(NS_OUTPUT::Object *o) = 0;
			virtual void finalize() = 0;
			std::string atomToString(int id, const ValueVector &values) const;
			bool addAtom(NS_OUTPUT::Atom *r);
			virtual int newUid();
			virtual ~Output();

			void hideAll();
			void setVisible(std::string *id, int arity, bool visible);
			bool isVisible(int uid);
			bool isVisible(std::string *id, int arity);
			
			// must be called if predicates are added after initialize has been called
			void addSignature();
		protected:
			int uids_;
			std::ostream *out_;
			AtomLookUp atoms_;
			SignatureVector *pred_;
			bool hideAll_;
			std::map<Signature, bool> hide_;
			std::vector<bool> visible_;
		};
	
		struct Object
		{
			Object();
			Object(bool neg);
			virtual void print(std::ostream &out) = 0;
			virtual void print_plain(std::ostream &out) = 0;
			virtual void addUid(Output *o) = 0;
			virtual void addDomain(bool fact = true) = 0;
			int getUid();
			virtual ~Object();
			
			bool neg_;
			int uid_;
		};

		struct Atom : public Object
		{
			Atom(bool neg, Node *node, int predUid, const ValueVector &values);
			Atom(bool neg, int predUid, const ValueVector &values);
			Atom(bool neg, int predUid);
			void addDomain(bool fact);
			void addUid(Output *o);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			
			// TODO: change sth here!
			Node *node_;
			int  predUid_;
			bool print_;
			Output *output_;
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
			Conjunction();
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			~Conjunction();
			void addDomain(bool fact);
			void addUid(Output *o);

			ObjectVector lits_;
		};

		struct Aggregate : public Object
		{
			enum Type   { SUM = 0xe, COUNT = 0xf, MAX = 0x10, MIN=0x11, TIMES=0x12, DISJUNCTION=0x13 };
			enum Bounds { LU = 3, U = 2, L = 1, N = 0 };
			Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights, int upper);
			Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights);
			Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights, int upper);
			Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights);
			Aggregate(bool neg, Type type);
			void print_plain(std::ostream &out);
			void print(std::ostream &out);
			void addDomain(bool fact);
			void addUid(Output *o);
			~Aggregate();

			int          type_;
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

			int          type_;
			ObjectVector lits_;
			IntVector    weights_;
		};	
	}
}

#endif

