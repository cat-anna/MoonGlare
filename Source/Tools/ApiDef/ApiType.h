#ifndef id0D47EB34_2ED4_4F77_BF3296D28C69B3D0
#define id0D47EB34_2ED4_4F77_BF3296D28C69B3D0

class ApiType : public ApiBaseType {
protected:
	ApiType(const std::string &Name, ApiBaseType *Parent, eSubclassType NodeType);
public:
	ApiType(const std::string &Name, ApiBaseType *Parent);
	
	void SaveToXML(pugi::xml_node node) const;
	void LoadFromXML(const pugi::xml_node node);
	
	virtual std::string ToString(unsigned Mode) const;
	virtual std::string CurrentPath() const;
	
	const ApiBaseType* FindType(const std::string &name, unsigned Flags) const;
};

#endif // header
