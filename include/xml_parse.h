#ifndef XML_PARSE_H
#define XML_PARSE_H

/**
 @file xml_parse.h
 A simple, callback-based xml parser
 
 @author James Gregson
 */


#include<ctype.h>
#include<cstdio>
#include<cstdarg>
#include<cstdlib>
#include<vector>
#include<string>

/**
    @brief Structure to hold user-callbacks for the xml-parser.  This is the
    mechanism by which data from the document is provided to the user.
*/
typedef struct {
    /** pointer set by the user which is passed as an argument to all callbacks */
    void *user_data;                                       
    
    /** called whenever a new tag is started */
    void (*begin_tag)( void *user_data, std::string &name );
    
    /** called whenever a tag is ended */
    void (*end_tag  )( void *user_data, std::string &name );
    
    /** called whenever the text for a tag is read */
    void (*tag_text )( void *user_data, std::string &text );
    
    /** called whenever a comment is read */
    void (*comment  )( void *user_data, std::string &comment );
    
    /** called whenever a tag attribute is read */ 
    void (*attribute)( void *user_data, std::string &name, std::string &value );
} xml_callbacks;

/**
    @brief Structure to hold the current state of the parser. Currently
    data is passed to the parser in the character array 'buffer'
    however this will likely change in subsequent versions.
*/
typedef struct {
    /** character buffer holding the raw xml data */
    std::string             buffer;
    
    /** integer character index for the xml stream, relative to the start of 'buffer' */
    int                     pos;
    
    /** line-number of position within the xml stream, determined by counting newlines */
    int                     line_number;
    
    /** column number of position within the xml stream, determined by counting characters */
    int                     column_number;
    
    /** pointer to the xml_callbacks structure which the parser uses to communicate with the user */
    xml_callbacks           *callbacks;
} xml_state;

/**
    @brief function to indicate whether the end of the stream has
    been reached
 
    @param[in] state Current parser state
*/
static inline bool xml_eof( xml_state *state ){
    return state->pos == (int)state->buffer.size();
}

/**
    @brief Error logging function for the xml-parser, dumps out the 
    file and line number (currently these are only done at 
    the current location, i.e. within the error function itself,
    which is kind of stupid). Then it throws an error to 
    halt further processing.
 
    @param[in]  fmt printf style formatting for variable argument list
 */
static void xml_error( const char *fmt, ... ){
    va_list args;
    va_start (args, fmt);
    fprintf( stderr, "Error in %s line %d: ", __FILE__, __LINE__ );
    vfprintf( stderr, fmt, args );
    va_end ( args );
    throw "xml_fatal_error";
}

/**
    Returns true if the character 'c' is a whitespace character
 
    @param[in]  c Input character to be tested
    @return true if c is a whitespace character, false otherwise
*/
static inline bool xml_is_space( char c ){
    return isspace( c );
}

/**
    Returns true if the character 'c' is a digit
 
    @param[in]  c Input character to be tested
    @return true if 'c' is a digit, false otherwise
 */
static inline bool xml_is_digit( char c ){
    return isdigit( c );
}

/**
    Returns true if the character 'c' is an alphabetic character
 
    @param[in]  c Input character to be tested
    @return true if 'c' is an alphabetic character false otherwise
 */
static inline bool xml_is_alpha( char c ){
    return isalpha( c );
}

/**
    returns true if the character 'c' is a valid character for a
    tag or attribute name, currently defined as either a letter,
    a digit or an underscore
 
    @param[in]  c Input character to be tested
    @return true if 'c' is a valid character for an xml name, 
            false otherwise
 */
static inline bool xml_is_valid_name_char( char c ){
    return xml_is_alpha(c) || xml_is_digit(c) || c == '_';
}

/**
    Peeks at and returns a character that is offset characters
    from the current stream position.
    
    @param[in]  state   Current parser state
    @param[in]  offset  Offset of character to return from current
                        stream position (default=0)
    @return Character value offset bytes from the current stream position
*/
static inline char xml_peek( xml_state *state, int offset=0 ){
    int fpos = state->pos+offset;
    if( fpos >= 0 && fpos < (int)state->buffer.size() )
        return state->buffer[state->pos+offset];
    xml_error("xml_peek(): tried to accces buffer index %d, valid range [0,%d]", fpos, state->buffer.size() );
    return '\0';
}

/**
    Advances the stream by a single character and tracks line
    and column number of the stream for error reporting
 
    @param[in] state Current parser state
*/
static inline void xml_advance( xml_state *state ){
    if( state->buffer[state->pos] == '\n' ){
        state->line_number++;
        state->column_number = 0;
    } else {
        state->column_number++;
    }
    state->pos++;
}

/**
    Ensures that the character at the current stream position 
    matches the character 'match'.  Prints an error and throws
    an exception if the character does not match, otherwise
    advances the character stream
 
    @param[in]  state   Current parser state
    @param[in]  match   Character to match
*/
static inline void xml_match( xml_state *state, char match ){
    char c = xml_peek( state );
    if( c != match ){
        xml_error( "xml_match(), expected %c, got %c at input line %d\n", match, c, state->line_number );
    }
    xml_advance( state );
}

/**
    Advances the input until a non-whitespace character is found
    
    @param[in]  state Current parser state
*/
static inline void xml_eat_space( xml_state *state ){
    while( !xml_eof(state) && xml_is_space( xml_peek( state ) ) ){
        xml_advance( state );
    }
}

/**
    Reads a quoted string from the input. Does not handle escape 
    characters in any way
 
    @param[in] state Current parser state
    @return returns the string that was read, without quotes
*/
static inline std::string xml_parse_string( xml_state *state ){
    std::string str;
    
    // gobble up whitespace
    xml_eat_space(state);
    
    // match the leading quote character
    xml_match(state, '\"');
    while( !xml_eof(state) && xml_peek(state) != '\"' ){        
        // add the current character to the end of 
        // the string being parsed
        str.push_back( xml_peek( state ) );
        xml_advance( state );
    }
    xml_match(state, '\"');
    str.push_back('\0');
    return str;
}

/**
    Reads an xml name (either a tag name or an attribute name)
    from the input. requires that the first non-whitespace
    character encountered is a letter
 
    @param[in]  state Current parser state
    @return name that was read
*/
static inline std::string xml_read_name( xml_state *state ){
    xml_eat_space(state);
    if( !xml_is_alpha( xml_peek( state ) ) ){
        xml_error( "xml_read_name(), expected an xml name, got '%c' at input line %d\n", xml_peek(state), state->line_number );
    }
    std::string name;   
    while( !xml_eof(state) && xml_is_valid_name_char( xml_peek( state ) ) ){
        name.push_back( xml_peek(state) );
        xml_advance(state);
    }
    return name;
}

/**
    Reads the text field for a tag by advancing the input
    until a '<' character is found. Returns the string
    that was read.
 
    @param[in] state    Current parser state
    @return text string that was read
*/
static inline std::string xml_read_text( xml_state *state ){
    std::string text;
    while( !xml_eof(state) && xml_peek( state ) != '<' ){
        text.push_back( xml_peek( state ) );
        xml_advance(state);
    }
    return text;
}

/**
    Reads a closing xml tag from the input i.e. &lt;/tag_name&gt;
    Returns the name that was read.
 
    @param[in]  state Current parser state
    @return tag name that was read
*/
static inline std::string xml_read_closing_tag( xml_state *state ){
    xml_match( state, '<' );
    xml_match( state, '/' );
    std::string name = xml_read_name( state );
    xml_match( state, '>' );
    return name;
}

/**
    Read an xml comment tag.  Returns the comment text.
    
    @param[in] state Current parser state
    @return comment string that was read
*/
static inline std::string xml_read_comment( xml_state *state ){
    std::string comment;
    
    // match the opening tag to the comment
    xml_match( state, '<' );
    xml_match( state, '!' );
    xml_match( state, '-' );
    xml_match( state, '-' );
    
    bool hyphen = false;
    while( !xml_eof(state) ){
        // current character is a hyphen, but the previous
        // character was not a hyphen, set the hyphen flag
        // and continue
        if( xml_peek( state ) == '-' && !hyphen ){
            xml_advance( state );
            hyphen = true;
            continue;
        }
        // current character is a hyphen and the previous
        // character was also a hyphe, this is only allowed
        // to denote the end of the comment, advance the
        // input by a character and match the closing '<'
        // and return
        if( xml_peek( state ) == '-' && hyphen ){
            xml_advance( state );
            xml_match( state, '>' );
            return comment;
        }
        
        // current character is not a hyphen, append the
        // character to the comment string and unset the 
        // hyphen flag
        comment.push_back( xml_peek( state ) );
        xml_advance( state );
        hyphen=false;
    }
    // never reached?
    return std::string("");
}

/**
    Matches the <?xml version="1.0" ... ?> header tag
 
    @param[in] state Current parser state
*/
static inline void xml_read_header( xml_state *state ){
    // match the header for the xml 'tag'
    xml_match(state, '<');
    xml_match(state, '?');
    xml_match(state, 'x');
    xml_match(state, 'm');
    xml_match(state, 'l');
    
    // read any attributes that occur
    while( !xml_eof(state) ){
        // each up whitespace
        xml_eat_space( state );
        
        // if the current character is alphabetic,
        // then try to read an xml attribute
        if( xml_is_alpha( xml_peek(state) ) ){
            xml_read_name(state);
            xml_eat_space(state);
            xml_match(state,'=');
            xml_parse_string(state);
            continue;
        }
        
        // if the current character is a question match,
        // match the end of the header tag
        if( xml_peek(state) == '?' && xml_peek(state,1) == '>' ){
            xml_match(state, '?');
            xml_match(state, '>');
            xml_eat_space(state);
            return;
        }
    }
}

/**
    Workhorse of the api, reads xml tags recursively, and 
    calls the functions in the xml_callbacks structure to
    allow the user's code to handle the data.
 
    @param[in]  state Current parser state
*/
static inline void xml_read_tag( xml_state *state ){
    std::string tag_name;
    std::string tag_text;
    std::string comment;
    
    // eat up all the leading whitespace
    xml_eat_space(state);
    
    // match the leading < character
    xml_match(state,'<');
    
    // make sure the next character is a letter
    tag_name = xml_read_name( state );
    state->callbacks->begin_tag( state->callbacks->user_data, tag_name );
    
    // read in the attributes
    while( !xml_eof(state) ){
        // eat
        xml_eat_space( state );
        
        if( xml_is_alpha( xml_peek( state ) ) ){
            // read the attribute name
            std::string attrib_name = xml_read_name(state);
            
            // eat any whitespace that may have been added
            xml_eat_space(state);
            
            // match the '=' character
            xml_match(state, '=');
            
            // read the attribute value
            std::string attrib_value = xml_parse_string(state);
            
            state->callbacks->attribute( state->callbacks->user_data, attrib_name, attrib_value );
            
            // go through the loop again
            continue;
        }
        
        // if the opening tag is being closed, break from this loop
        if( xml_peek(state) == '>' ){
            xml_match( state, '>' );
            break;
        }
        
        // if the whole tag is being closed, break from this loop
        if( xml_peek(state) == '/' && xml_peek(state,1) == '>' ){
            xml_match( state, '/' );
            xml_match( state, '>' );
            state->callbacks->end_tag( state->callbacks->user_data, tag_name );
            xml_eat_space(state);
            return;
        }
    }
    
    while( !xml_eof(state) ){
        // eat whitespace
        xml_eat_space( state );
        
        // try to read a closing tag
        if( xml_peek(state) == '<' && xml_peek(state,1) == '/' ){
            std::string close_name = xml_read_closing_tag( state );
            if( close_name.compare( tag_name ) != 0 ){
                xml_error( "xml_read_tag(), expected closing name (%s) to match tag name (%s) at input line %d\n", close_name.c_str(), tag_name.c_str(), state->line_number );
            }
            state->callbacks->end_tag( state->callbacks->user_data, tag_name );
            break;
        }
        
        // try to read a comment
        if( xml_peek(state) == '<' && xml_peek(state,1) == '!' ){
            comment = xml_read_comment( state );
            state->callbacks->comment( state->callbacks->user_data, comment );
            continue;
        }
        
        // try to read a child tag
        if( xml_peek(state) == '<' && xml_peek(state,1) != '/' ){
            xml_read_tag( state );
            continue;
        }
        
        // try to read the text of the tag (if applicable)
        tag_text = xml_read_text( state );
        state->callbacks->tag_text( state->callbacks->user_data, tag_text );
    }
}

/**
 @brief reads an xml document by first trying to read the 
 xml header, followed by recursively reading any tags that
 occur
 
 @param[in] state Current parser state
 */
static inline void xml_read_document( xml_state *state ){
    // eat any whitespace that occurs
    xml_eat_space( state );
    
    bool first = true;
    while( !xml_eof(state) ){
        if( xml_peek(state)=='<' ){
            if( xml_peek(state,1)=='?' ){
                if( !first ){
                    xml_error("encountered a header tag midway through file");
                }
                // read the xml-header information
                xml_read_header( state );
            } else if( xml_peek(state,1)=='!'){
                // found a comment tag
                std::string comment = xml_read_comment( state );
                state->callbacks->comment( state->callbacks->user_data, comment );
            } else if( xml_is_alpha( xml_peek(state,1) ) ){
                // alphabetic character, read a tag
                xml_read_tag(state);
            }
            xml_eat_space(state);
        } else {
            xml_error("expected a < character\n");
        }
    }
}

#endif
