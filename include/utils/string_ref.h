


#pragma  once
#include <string>
#include <vector>
class StringRef
{
private:
	std::string::const_iterator     begin_;
	size_t             size_;
public:
	size_t size() const { return size_; }
	std::string::const_iterator begin() const { return begin_; }
	std::string::const_iterator end() const { return begin_ + size_; }

	StringRef(const std::string::const_iterator & begin, const size_t size)
		: begin_(begin)
		, size_(size)
	{}


	StringRef(const std::string::const_iterator & begin, const std::string::const_iterator & end)
			: begin_(begin)
			, size_(end - begin)
	{}
};


std::ostream & operator<<(std::ostream & out, const StringRef & ref){
	std::cout << std::string(ref.begin(), ref.end());
	return out;
}