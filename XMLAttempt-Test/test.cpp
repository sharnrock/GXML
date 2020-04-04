#include "gtest/gtest.h"


const char* xml_little_prolog =
"<?xml version=\"1.0\" encoding=\"UTF - 8\" standalone=\"no\" ?>"
"<root></root>";

const char* xml_little_prolog_whitespace =
"< ? xml version = \"1.0\" encoding = \"UTF - 8\" standalone = \"no\" ? >"
"<root></root>";

const char* xml_sample =
"<?xml version=\"1.0\" encoding=\"UTF - 8\" standalone=\"no\" ?>"
"<breakfast_menu>"
"<food>"
"<name>Belgian Waffles< / name>"
"<price>$5.95 < / price >"
"<description>"
"Two of our famous Belgian Waffles with plenty of real maple syrup"
"< / description>"
"< calories>650 < / calories >"
"< / food>"
"<food>"
"<name>Strawberry Belgian Waffles< / name>"
"<price>$7.95 < / price >"
"<description>"
"Light Belgian waffles covered with strawberriesand whipped cream"
"< / description>"
"< calories>900 < / calories >"
"< / food>"
"<food>"
"<name>Berry - Berry Belgian Waffles< / name>"
"<price>$8.95 < / price >"
"<description>"
"Light Belgian waffles covered with an assortment of fresh berriesand whipped cream"
"< / description>"
"< calories>900 < / calories >"
"< / food>"
"<food>"
"<name>French Toast< / name>"
"<price>$4.50 < / price >"
"<description>"
"Thick slices made from our homemade sourdough bread"
"< / description>"
"< calories>600 < / calories >"
"< / food>"
"<food>"
"<name>Homestyle Breakfast< / name>"
"<price>$6.95 < / price >"
"<description>"
"Two eggs, bacon or sausage, toast, and our ever - popular hash browns"
"< / description>"
"<calories>950 </calories>"
"< / food>"
"< / breakfast_menu>";

#include "GString.h"
#include "GMap.h"
#include <stdexcept>

namespace G
{
    namespace XMLExceptions
    {
        class EncodingNotYetImplemented : public std::exception
        {
        public:
            char const* what() const
            {
                return "That character encoding has not yet been implemented, data garbling will occur";
            }
        };

        class XMLParseException : public std::exception
        {
        public:
            XMLParseException(const char* current_looky_pointer)
                : broke_here(current_looky_pointer)
            {}
        protected:
            const char* broke_here;
        };

        class InvalidProlog : public XMLParseException
        {
        public:
            InvalidProlog(const char* current_looky_pointer) 
                : XMLParseException(current_looky_pointer)
            {}

            char const* what() const
            {
                return "Invalid Prolog";
            }
        };

        class MissingTagEndingBracket : public XMLParseException
        {
        public:
            MissingTagEndingBracket(const char* current_looky_pointer)
                : XMLParseException(current_looky_pointer)
            {}

            char const* what() const
            {
                return "Missing '>' tag ending bracket";
            }
        };
    }

    class XMLTag
    {
    public:
        XMLTag() {}
        XMLTag(const char* name)
            : name(name)
        {}

        bool isNull() const { return name.isNull(); }
        void setName(const char* name) { this->name = name; }
        const char* getName() const { return name.toConstChar(); }
        // TODO: add attributes
        // TODO: create tag assignment oeprator
    private:
        String name;
    };

    class XMLStringParser
    {
        // Start over with new testing
    public:
        XMLTag* getRoot()
        {
            if (!is_parsed)
                parse();
            return &xml_root;
        }

    private:
        XMLTag parseTag(const char*& current_looky_ptr)
        {
            List<char> tag_name;
            while (!isWhiteSpace(*current_looky_ptr) && *current_looky_ptr != '>')
            {
                tag_name.append(*current_looky_ptr++);
            }
            tag_name.append('\0');
            if (xml_root.isNull())
            {
                xml_root.setName(tag_name.getData());
                return xml_root; // TODO: create tag assignment oeprator & replace the set name function!
            }
            else
            {
                return XMLTag(tag_name.getData());
            }
        }


    public:
        XMLStringParser(const char* _xml) 
            : xml(_xml)
            , is_parsed(false)
            , xml_version("0.0")
        {
        }

        const char* getVersion() 
        { 
            if (!is_parsed)
                parse();
            return xml_version.toConstChar(); 
        }


        

        void parse()
        {
            // TODO: should always check to see if prolog exists
            // TODO: then check for root bracket

            const char* current_looky_ptr = xml.toConstChar();
            switch (*current_looky_ptr)
            {
            case '<':
                parseBracketedThing(current_looky_ptr);
                break;
            default:
                parseUninterestingThing(current_looky_ptr);
                break;
            }
        }

        void parseBracketedThing(const char*& current_looky_ptr)
        {
            skipWhiteSpace(current_looky_ptr);
                
            switch (*current_looky_ptr)
            {
            case '!':
                skipComment(current_looky_ptr);
                break;
            case '?':
                parseProlog(current_looky_ptr);
                break;
            default:
                parseTag(current_looky_ptr);
                break;
            }
        }



        void skipComment(const char*& current_looky_ptr)
        {
            throw std::runtime_error("Not implemnted comment skip");
        }

        void skipWhiteSpace(const char*& current_looky_ptr)
        {
            do {
                ++current_looky_ptr;
            } while (isWhiteSpace(*current_looky_ptr));
        }

        void parseProlog(const char*& current_looky_ptr)
        {
            skipWhiteSpace(current_looky_ptr);
            const char* expected = "xml";
            for (int i = 0; i < 4; i++)
            {
                if (expected[i] != *current_looky_ptr++)
                    throw XMLExceptions::InvalidProlog(current_looky_ptr);
            }
            skipWhiteSpace(current_looky_ptr);

            switch (*current_looky_ptr)
            {
            case '?':
            {
                skipWhiteSpace(current_looky_ptr);
                if (*current_looky_ptr != '>')
                    throw XMLExceptions::MissingTagEndingBracket(current_looky_ptr);
            }
            break;
            default:
            {
                Map<String, String> attribute_map = parseAttributesIntoMap(current_looky_ptr);
                xml_version = attribute_map["version"].toConstChar();
                xml_encoding = attribute_map["encoding"].toConstChar();
            }
            }
            
            // collect attributes (in order?)
            // version, encoding
        }

        Map<String, String> parseAttributesIntoMap(const char*& current_looky_ptr)
        {
            Map<String, String> result;
            skipWhiteSpace(current_looky_ptr);
            while (*current_looky_ptr != '?' && *current_looky_ptr != '>')
            {
                if (*current_looky_ptr == '\0')
                    throw std::runtime_error("Reached end of string without ending attribute key");

                G::List<char> key;
                G::List<char> value;
                bool is_key = true;
                if (is_key)
                {
                    key.append(*current_looky_ptr++); // Add to actual key string

                    if (isWhiteSpace(*current_looky_ptr)) // Detect if we've made it to the end of the key
                    {
                        skipWhiteSpace(current_looky_ptr);
                        if (*current_looky_ptr++ != '=')
                            throw std::runtime_error("Missing equal sign in attribute");
                        is_key = false;
                        skipWhiteSpace(current_looky_ptr); // bring us to the front of the value
                        if (*current_looky_ptr++ != '"')
                            throw std::runtime_error("Missing opening quote");
                    }
                    else if (*current_looky_ptr == '=') // Detect if we've made it to the end of the key
                    {
                        is_key = false;
                        ++current_looky_ptr; // skip '='
                        skipWhiteSpace(current_looky_ptr); // bring us to the front of the value
                        if (*current_looky_ptr++ != '"')
                            throw std::runtime_error("Missing opening quote");
                    }
                    
                }
                else
                {
                    // By the time we get here, we should be into the meat of the value, not the opening quote or anything else
                    value.append(*current_looky_ptr++);
                    if (*current_looky_ptr == '"')
                    {
                        result[key.getData()] = value.getData();
                        // TODO: finish
                    }
                    else if (*current_looky_ptr == '\0')
                    {
                        throw std::runtime_error("Made it to the end of the string without closing quote for value");
                    }

                }


                // TODO: put that stuff in the map
            }

            return result; 
        }

        bool isWhiteSpace(char c)
        {
            if (c == ' ')
                return true;
            else if (c == '\t')
                return true;
            else if (c == '\r')
                return true;
            else if (c == '\n')
                return true;
            return false;
        }

        void parseUninterestingThing(const char*& current_looky_ptr)
        {
            ++current_looky_ptr;
        }

    private:
        String xml;
        XMLTag xml_root;
        String xml_version;
        String xml_encoding;
        bool is_parsed;
    };
}

TEST(GXMLTag, AssignmentOperator_DoesSOmething_When_Something)
{
    // TODO: continue from here
    // add everything to xml tag class that needs to be done
    // Then go back and make sure you can parse it from string parser class
    ASSERT_FALSE(true);
}

TEST(GXML, GetRoot_ReturnsRootString_When_RootExists)
{
    const char* expected = "root";
    const char* xml = "<root></root>";
    G::XMLStringParser parser(xml);
    const char* actual = parser.getRoot()->getName();
    ASSERT_STREQ(expected, actual);
}

TEST(GXML, GetVersion_ParsePrologVersion_When_NoErrors) 
{
    const char* expected = "1.0";
    G::XMLStringParser xml(xml_little_prolog);
    std::string actual(xml.getVersion());
    ASSERT_STREQ(expected, actual.c_str());
}

// TODO: throws invalid prolog if error
// TODO: missing tag ending bracket
// TODO: grab encoding