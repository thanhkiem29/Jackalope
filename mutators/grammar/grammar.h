#include <string>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>

#pragma once

class PRNG;
class Sample;

#define BINARY_RW_INITIAL_SIZE 128

#define MAX_DEPTH 100

#define REPEAT_PROBABILITY (0.75)

class Grammar {
protected:

  // A small helper class for serializing
  // grammar-generated representation to/from
  // binary form
  class BinaryRW {
  public:
    BinaryRW();
    BinaryRW(size_t size, unsigned char* data);
    ~BinaryRW();

    void WriteString(std::string* s);
    void WriteSize(size_t size);
    void WriteData(unsigned char* data, size_t size);
    int ReadString(std::string* s);
    int ReadSize(size_t* size);
    int ReadData(unsigned char* data, size_t size);

    size_t GetSize() { return size_current; }
    unsigned char* GetData() { return bytes; }

  protected:
    size_t size_allocated;
    size_t size_current;
    size_t read_pos; //for reading
    unsigned char* bytes;
  };

  // Parser state
  enum ParseState {
    LINESTART,
    GENERATORSYMBOL,
    GENERATORSYMBOLEND,
    EQUAL,
    EQUALSPACE,
    SYMBOL,
    SYMBOLEND,
    STRING
  };

public:

  // Common grammar structures

  enum NodeType {
    STRINGTYPE = 0,
    SYMBOLTYPE = 1
  };

  class Symbol;

  class RulePart {
  public:
    NodeType type;
    std::string value;
    Symbol* symbol;
  };

  class Rule {
  public:
    std::string generates;
    std::vector<RulePart> parts;
  };

  class Symbol {
  public:
    Symbol(std::string name) : name(name), repeat(0), can_be_empty(0), used(false) { }
    int repeat;
    int can_be_empty;
    Symbol* repeat_symbol;
    std::string name;
    std::vector<Rule> generators;
    bool used;
  };

  // Grammar::TreeNode is a basic building block
  // of a sample generated by grammar
  class TreeNode {
  public:
    TreeNode() : type(STRINGTYPE), string(NULL) {}
    TreeNode(const TreeNode& other);
    TreeNode& operator=(const TreeNode& other);
    // Note: other gets deleted
    void Replace(TreeNode* other);
    ~TreeNode();
    void Clear();

    size_t NumNodes();

    NodeType type;
    union {
      Symbol* symbol;
      std::string *string;
    };
    std::vector<TreeNode*> children;
  };

public:
  Grammar();

  // reads grammar from a text file
  int Read(const char* filename);

  // generates tree starting with a given root symbol
  TreeNode* GenerateTree(const char* symbol, PRNG* prng);
  TreeNode* GenerateTree(Symbol* symbol, PRNG* prng, int depth = 0);

  // Converts a tree into a string representation
  void ToString(TreeNode* tree, std::string& out);

  // Retrieves a Symbol by name
  Symbol* GetSymbol(std::string& name);

  // Encodes/decodes a tree to/from binary form
  void EncodeSample(TreeNode* tree, Sample* sample);
  TreeNode* DecodeSample(Sample* sample);

protected:
  TreeNode* GenerateStringNode(std::string* string);
  int GenerateString(const char* symbol, PRNG* prng, std::string* out);

  void AnalyzeGrammar();

  std::string* GetStringFromCache(std::string& s);

  int ParseGrammarLine(std::string& line, int lineno);
  int CheckGrammar();

  Symbol* GetOrCreateSymbol(std::string& name);

  int AddRulePart(Rule* rule, NodeType type, std::string& value);
  int HexStringToString(std::string& hex, std::string& out);

  void EncodeTree(TreeNode* tree, BinaryRW* rw);
  TreeNode* DecodeTree(BinaryRW* rw);

  std::unordered_map<std::string, Symbol*> symbols;
  std::map<std::string, std::string> constants;

  std::unordered_map<std::string, std::string*> string_cache;
};
