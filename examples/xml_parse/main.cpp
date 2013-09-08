#include<cstdio>
#include<iostream>

#include"../../include/xml_parse.h"

// usage instructions (Unix/OS-X)
// compile with 'g++ main.cpp -o test', run with './test'

// =========================================================================
// helper functions, not specific to xml_parse

// only used to format output
void print_scope( int scope );

// helper function to load a file into a string
bool load_file_into_string( const char *filename, std::string &buffer );


// =========================================================================
// callbacks needed for parsing, see below for implementation

// callback called when a new tag is found
void begin_tag( void *user_data, std::string &name );

// callback called when a tag is finished
void end_tag( void *user_data, std::string &name );

// callback called when tag text is read
void tag_text( void *user_data, std::string &text );

// callback called when a comment is encountered
void comment( void *user_data, std::string &comment );

// callback called when a tag attribute is read
void attribute( void *user_data, std::string &name, std::string &value );

// =========================================================================
// entry point
int main( int argc, char **argv ){
    // load the input file into a string
    std::string buffer;
    load_file_into_string( "../test.xml", buffer );

    // setup the callbacks and user data that will be used
    // during parsing, then define the xml parser state
    int scope = 0;
    xml_callbacks callbacks = { &scope, begin_tag, end_tag, tag_text, comment, attribute };
    xml_state state = { buffer, 0, 0, 0, &callbacks };
    
    // read in the document, callbacks defined below should now print
    // formatted output to stdout
    xml_read_document( &state );
    
    return 0;
}

// =========================================================================
// callback implementations
void begin_tag( void *user_data, std::string &name ){
    int *scope = (int*)user_data;
    print_scope( *scope );
    printf( "BEGIN TAG: %s\n", name.c_str() );
    (*scope)++;
}

void end_tag( void *user_data, std::string &name ){
    int *scope = (int*)user_data;
    (*scope)--;
    print_scope( *scope );
    printf( "END TAG: %s\n", name.c_str() );
}

void tag_text( void *user_data, std::string &text ){
    int *scope = (int*)user_data;
    print_scope( *scope );
    printf( "TEXT: %s\n", text.c_str() );
}

void comment( void *user_data, std::string &comment ){
    int *scope = (int*)user_data;
    print_scope( *scope );
    printf( "COMMENT: %s\n", comment.c_str() );
}

void attribute( void *user_data, std::string &name, std::string &value ){
    int *scope = (int*)user_data;
    print_scope( *scope );
    printf( "ATTRIBUTE: %s=%s\n", name.c_str(), value.c_str() );
}

// =========================================================================
// auxilliary routines
void print_scope( int scope ){
    for( int i=0; i<scope; i++ ){
        printf("  ");
    }
}

bool load_file_into_string( const char *filename, std::string &buffer ){
    FILE *fp = fopen( filename, "r" );
    if(!fp){
        printf("Error opening file %s\n", filename );
        return false;
    }
    long size;
    
    fseek( fp, 0, SEEK_END );
    size = ftell(fp);
    rewind(fp);
    
    buffer.resize( size );
    fread( &buffer[0], 1, size, fp );
    fclose(fp);
    return true;
}
