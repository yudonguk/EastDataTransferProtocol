#pragma once

#include <boost/noncopyable.hpp>
#include <boost/property_tree/json_parser.hpp>

class JsonHelper : private boost::noncopyable
{
public:
	  /**
     * Translates the property tree to JSON and writes it the given output
     * stream.
     * @note Any property tree key containing only unnamed subkeys will be
     *       rendered as JSON arrays.
     * @pre @e pt cannot contain keys that have both subkeys and non-empty data.
     * @throw json_parser_error In case of error translating the property tree
     *                          to JSON or writing to the output stream.
     * @param stream The stream to which to write the JSON representation of the
     *               property tree.
     * @param pt The property tree to tranlsate to JSON and output.
     * @param pretty Whether to pretty-print. Defaults to true for backward
     *               compatibility.
     */
    template<class Ptree>
    static void write_json(std::basic_ostream<
                        typename Ptree::key_type::value_type
                    > &stream,
                    const Ptree &pt,
                    bool pretty = true)
    {
        write_json_internal(stream, pt, std::string(), pretty);
    }

    /**
     * Translates the property tree to JSON and writes it the given file.
     * @note Any property tree key containing only unnamed subkeys will be
     *       rendered as JSON arrays.
     * @pre @e pt cannot contain keys that have both subkeys and non-empty data.
     * @throw json_parser_error In case of error translating the property tree
     *                          to JSON or writing to the file.
     * @param filename The name of the file to which to write the JSON
     *                 representation of the property tree.
     * @param pt The property tree to translate to JSON and output.
     * @param loc The locale to use when writing out to the output file.
     * @param pretty Whether to pretty-print. Defaults to true and last place
     *               for backward compatibility.
     */
    template<class Ptree>
    static void write_json(const std::string &filename,
                    const Ptree &pt,
                    const std::locale &loc = std::locale(),
                    bool pretty = true)
    {
        std::basic_ofstream<typename Ptree::key_type::value_type>
            stream(filename.c_str());
        if (!stream)
            BOOST_PROPERTY_TREE_THROW(json_parser_error(
                "cannot open file", filename, 0));
        stream.imbue(loc);
        write_json_internal(stream, pt, filename, pretty);
    }


	// Create necessary escape sequences from illegal characters
	template<class Ch>
	static std::basic_string<Ch> create_escapes(const std::basic_string<Ch> &s)
	{
		std::basic_string<Ch> result;
		typename std::basic_string<Ch>::const_iterator b = s.begin();
		typename std::basic_string<Ch>::const_iterator e = s.end();
		while (b != e)
		{
			// This assumes an ASCII superset. But so does everything in PTree.
			// We escape everything outside ASCII, because this code can't
			// handle high unicode characters.
			if (*b == 0x20 || *b == 0x21 || (*b >= 0x23 && *b <= 0x2E) ||
				(*b >= 0x30 && *b <= 0x5B) || (*b >= 0x5D && *b <= 0xFF))
				result += *b;
			else if (*b == Ch('\b')) result += Ch('\\'), result += Ch('b');
			else if (*b == Ch('\f')) result += Ch('\\'), result += Ch('f');
			else if (*b == Ch('\n')) result += Ch('\\'), result += Ch('n');
			else if (*b == Ch('\r')) result += Ch('\\'), result += Ch('r');
			else if (*b == Ch('/')) result += Ch('\\'), result += Ch('/');
			else if (*b == Ch('"'))  result += Ch('\\'), result += Ch('"');
			else if (*b == Ch('\\')) result += Ch('\\'), result += Ch('\\');
			else
			{
				const char *hexdigits = "0123456789ABCDEF";
				typedef boost::detail::make_unsigned_imp<Ch>::type UCh;
				unsigned long u = (std::min)(static_cast<unsigned long>(
					static_cast<UCh>(*b)),
					0xFFFFul);
				int d1 = u / 4096; u -= d1 * 4096;
				int d2 = u / 256; u -= d2 * 256;
				int d3 = u / 16; u -= d3 * 16;
				int d4 = u;
				result += Ch('\\'); result += Ch('u');
				result += Ch(hexdigits[d1]); result += Ch(hexdigits[d2]);
				result += Ch(hexdigits[d3]); result += Ch(hexdigits[d4]);
			}
			++b;
		}
		return result;
	}

	template<class Ptree>
	static void write_json_helper(std::basic_ostream<typename Ptree::key_type::value_type> &stream, 
		const Ptree &pt,
		int indent, bool pretty)
	{

		typedef typename Ptree::key_type::value_type Ch;
		typedef typename std::basic_string<Ch> Str;

		// Value or object or array
		if (indent > 0 && pt.empty())
		{
			// Write value
			Str data = create_escapes(pt.template get_value<Str>());

			if (data.empty())
			{
				stream << Ch('"') << Ch('"');
			}
			else if(*data.begin() == Ch('\\') && *data.rbegin() == Ch('"'))
			{
				stream << Ch('"') << data.substr(2 , data.size() - 4) << Ch('"');
			}
			else
			{
				stream << data;
			}
		}
		else if (indent > 0 && pt.count(Str()) == pt.size())
		{
			// Write array
			stream << Ch('[');
			if (pretty) stream << Ch('\n');
			typename Ptree::const_iterator it = pt.begin();
			for (; it != pt.end(); ++it)
			{
				if (pretty) stream << Str(4 * (indent + 1), Ch(' '));
				write_json_helper(stream, it->second, indent + 1, pretty);
				if (boost::next(it) != pt.end())
					stream << Ch(',');
				if (pretty) stream << Ch('\n');
			}
			stream << Str(4 * indent, Ch(' ')) << Ch(']');

		}
		else
		{
			// Write object
			stream << Ch('{');
			if (pretty) stream << Ch('\n');
			typename Ptree::const_iterator it = pt.begin();
			for (; it != pt.end(); ++it)
			{
				if (pretty) stream << Str(4 * (indent + 1), Ch(' '));
				stream << Ch('"') << create_escapes(it->first) << Ch('"') << Ch(':');
				if (pretty) {
					if (it->second.empty())
						stream << Ch(' ');
					else
						stream << Ch('\n') << Str(4 * (indent + 1), Ch(' '));
				}
				write_json_helper(stream, it->second, indent + 1, pretty);
				if (boost::next(it) != pt.end())
					stream << Ch(',');
				if (pretty) stream << Ch('\n');
			}
			if (pretty) stream << Str(4 * indent, Ch(' '));
			stream << Ch('}');
		}

	}

	// Verify if ptree does not contain information that cannot be written to json
	template<class Ptree>
	static bool verify_json(const Ptree &pt, int depth)
	{

		typedef typename Ptree::key_type::value_type Ch;
		typedef typename std::basic_string<Ch> Str;

		// Root ptree cannot have data
		if (depth == 0 && !pt.template get_value<Str>().empty())
			return false;

		// Ptree cannot have both children and data
		if (!pt.template get_value<Str>().empty() && !pt.empty())
			return false;

		// Check children
		typename Ptree::const_iterator it = pt.begin();
		for (; it != pt.end(); ++it)
			if (!verify_json(it->second, depth + 1))
				return false;

		// Success
		return true;

	}

	// Write ptree to json stream
	template<class Ptree>
	static void write_json_internal(std::basic_ostream<typename Ptree::key_type::value_type> &stream, 
		const Ptree &pt,
		const std::string &filename,
		bool pretty)
	{
		if (!verify_json(pt, 0))
			BOOST_PROPERTY_TREE_THROW(boost::property_tree::json_parser::json_parser_error("ptree contains data that cannot be represented in JSON format", filename, 0));
		write_json_helper(stream, pt, 0, pretty);
		stream << std::endl;
		if (!stream.good())
			BOOST_PROPERTY_TREE_THROW(boost::property_tree::json_parser::json_parser_error("write error", filename, 0));
	}

	template<typename StringType>
	struct StringTranslator
	{
		//typedef T internal_type;
		//typedef T external_type;

		template <typename T>
		boost::optional<T> get_value(const StringType& v) { return  v.substr(1, v.size() - 2); }

		boost::optional<StringType> put_value(const std::wstring& v) { return L'"' + v + L'"'; }

		boost::optional<StringType> put_value(const std::string& v) { return '"' + v + '"'; }
	};

	template<typename TreeType, typename PathType, typename ValueType>
	static TreeType& Put(TreeType& tree, const PathType& path, const ValueType& value)
	{
		return tree.put(path, value);
	}
	
	template<typename TreeType, typename PathType>
	static TreeType& Put(TreeType& tree, const PathType& path, const std::string& value)
	{
		return tree.put(path, value, StringTranslator<std::string>());
	}

	template<typename TreeType, typename PathType>
	static TreeType& Put(TreeType& tree, const PathType& path, const std::wstring& value)
	{
		return tree.put(path, value, StringTranslator<std::wstring>());
	}
};