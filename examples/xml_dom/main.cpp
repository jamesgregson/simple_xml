#include<cstdio>
#include<string>
#include<iostream>
#include"../../include/xml_dom.h"

// usage instructions (Unix/OS-X)
// compile with 'g++ main.cpp -o test', run with './test'

// forward declaration of a file-reading function
bool load_file_into_string( const char *filename, std::string &buffer );

int main( int argc, char **argv ){
    // load the example file into a buffer for reading
    std::string buffer;
    load_file_into_string( "../test.xml", buffer );
    
    // parse the data from the buffer, then locate the correspondence entity 
    // within the DOM.
    xml_dom_entity *doc = xml_dom_parse( buffer );
    xml_dom_entity *corr = doc->first_child_tag("root")->first_child_tag("correspondence");
    
    // Dump the xml data rooted at the corr tag to cout
    std::cout << "==============================================================" << std::endl;
    std::cout << *corr;
    
    // now add a tag after the correspondence tag and dump out the result
    xml_dom_entity *subtag = corr->add_tag("newtag");
    subtag->add_attribute("attrib0","value0");
    subtag->add_attribute("attrib1","value1");
    subtag->set_value("newtag value");
    
    // now dump out the result again to show the changes
    std::cout << "==============================================================" << std::endl;
    std::cout << *corr;
    
    // free up the memory used by the DOM
    delete doc;
    
    return 0;
}

bool load_file_into_string( const char *filename, std::string &buffer ){
    long size;
    FILE *fp = fopen( filename, "r" );
    if(!fp){
        printf("Error opening file %s\n", filename );
        return false;
    }    
    fseek( fp, 0, SEEK_END );
    size = ftell(fp);
    rewind(fp);
    
    buffer.resize( size );
    fread( &buffer[0], 1, size, fp );
    fclose(fp);
    return true;
}