#ifndef XML_DOM_H
#define XML_DOM_H

#include<list>
#include<string>
#include<vector>
#include<cassert>
#include<algorithm>
#include<iostream>

#include"xml_parse.h"

/**
    @file xml_dom.h
    A slightly higher level interface to an xml-document
    than the corresponding one provided by xml_parse.h.
    
    This file provides an implementation of a parser
    which builds a tree-based representation of the 
    document and is more flexible to navigate.
*/

/**
    @brief enumeration to indicate the different xml
    entity types to use when building the document tree
*/
typedef enum {
    XML_DOM_INVALID,
    XML_DOM_DOCUMENT,
    XML_DOM_TAG,
    XML_DOM_ATTRIBUTE,
    XML_DOM_COMMENT,
} xml_dom_entity_type;

/**
    @brief base class for all xml entities in the document.
    Every entity type in the xml_dom_entity_type enumeration
    (with the exception of the INVALID type) is represented as
    a name and optional value and optional child elements. This
    is effectively the same as transforming all tag attributes
    to be child tags.  These different child tags are distinguished
    by their m_type member variables, and store their positions
    within their parent tag's child arrays.
*/
class xml_dom_entity {
    friend std::ostream& operator<<(std::ostream& output, xml_dom_entity &p);
private:
    /** type of the entity, can be document, tag, attribute or comment */
    xml_dom_entity_type             m_type;
    
    /** parent entity of the current entity */
    xml_dom_entity                  *m_parent;
    
    /** array of child entities of the current entity */
    std::vector<xml_dom_entity*>    m_child;
    
    /** index within m_parent's m_child list of the current entity, to
        allow next/previous child/sibling queries without having to 
        search for the current tag
    */
    int                             m_index;
    
    /** name of the entity, this is non-existent for DOCUMENT and
        COMMENT types.  For TAG types, it is the name immediately 
        following the opening < of the tag definition.  For
        ATTRIBUTE type entities it is the attribute name */
    std::string                     m_name;
    
    /** 'value' of the entity, as follows for the various types:
            DOCUMENT  - n/a
            TAG       - the text of the tag
            ATTRIBUTE - the value of the attribute
            COMMENT   - the comment text
    */
    std::string                     m_value;
public:
    /**
     Default constructor, initializes the entity to be invalide
    */
    xml_dom_entity(){
        m_type = XML_DOM_INVALID;
        m_index = -1;
        m_parent = NULL;
    }
    
    /**
     recursively free any memory allocated while constructing the DOM
    */
    ~xml_dom_entity(){
        for( int i=0; i<(int)m_child.size(); i++ ){
            delete m_child[i];
        }
    }
    
    /**
     returns type type of the entity
    */
    inline xml_dom_entity_type get_type(){
        return m_type;
    }
    
    /**
     sets the type of the entity
    */
    inline void set_type( xml_dom_entity_type type ){
        m_type=type;
    }
    
    /**
     returns the parent of the entry
    */
    inline xml_dom_entity *get_parent(){
        assert( m_type != XML_DOM_INVALID );
        return m_parent;
    }
    
    /**
     sets the parent of the entry
    */
    inline void set_parent( xml_dom_entity *parent ){
        assert( m_type != XML_DOM_INVALID );        
        m_parent = parent;
    }
    
    /**
     returns the number of children of the entity
    */
    inline int num_children(){
        assert( m_type != XML_DOM_INVALID );
        return m_child.size();
    }
    
    /**
     returns the child at index 'index'
    */
    inline xml_dom_entity *get_child( int index ){
        assert( m_type != XML_DOM_INVALID );
        assert( index >= 0 && index < num_children() );
        return m_child[index];
    }
    
    /**
     Convenience method to add a new tag as a child to the current
     tag or document.  Can also be done with add_child manually.
    */
    inline xml_dom_entity *add_tag( std::string name ){
        assert( m_type == XML_DOM_TAG || m_type == XML_DOM_DOCUMENT );
        xml_dom_entity *tag = new xml_dom_entity();
        tag->set_type( XML_DOM_TAG );
        tag->set_name( name );
        add_child( tag );
        return tag;
    }
    
    /**
     convenience method to add a new attribute to a tag or document
     declaration, can also be done with add child, provided the 
     user builds the attribute tag themselves
    */
    inline xml_dom_entity *add_attribute( std::string name, std::string value ){
        assert( m_type == XML_DOM_TAG || m_type == XML_DOM_DOCUMENT );
        xml_dom_entity *attr = new xml_dom_entity();
        attr->set_type( XML_DOM_ATTRIBUTE );
        attr->set_name( name );
        attr->set_value( value );
        add_child( attr );
        return attr;
    }
    
    /**
     convenience method to add a new comment tag as a child to the 
     current document
    */
    inline xml_dom_entity *add_comment( std::string comment ){
        assert( m_type == XML_DOM_TAG || m_type == XML_DOM_DOCUMENT );
        xml_dom_entity *comm = new xml_dom_entity();
        comm->set_type( XML_DOM_COMMENT );
        comm->set_value( comment );
        add_child( comm );
        return comm;
    }
    
    /**
     adds an existing child to the entity
    */
    inline void add_child( xml_dom_entity *child ){
        assert( m_type != XML_DOM_INVALID );
        child->m_parent = this;
        child->m_index = num_children();
        m_child.push_back( child );
    }
    
    /**
     returns the name of the entity
    */
    inline std::string get_name(){
        assert( m_type != XML_DOM_INVALID );
        return m_name;
    }
    
    /**
     sets the name of the entity
    */
    inline void set_name( std::string name ){
        assert( m_type != XML_DOM_INVALID );
        m_name = name;
    }
    
    /**
     returns the name of the entity
     */
    inline std::string get_value(){
        assert( m_type != XML_DOM_INVALID );
        return m_value;
    }
    
    /**
     sets the name of the entity
     */
    inline void set_value( std::string value ){
        assert( m_type != XML_DOM_INVALID );
        m_value = value;
    }
    
    /**
     returns the first child (which may be a tag, comment or
     attribute) of this entity
    */
    inline xml_dom_entity *first_child(){
        if( num_children() > 0 )
            return get_child(0);
        return NULL;
    }
    
    /**
     returns the first child element of this entity whose type
     matches 'type'
    */
    inline xml_dom_entity *first_child( xml_dom_entity_type type ){
        if( num_children() > 0 ){
            xml_dom_entity *child = first_child();
            while( child ){
                if( child->m_type == type )
                    return child;
                child = next_child( child );
            }
            return NULL;
        }
        return NULL;
    }
    
    /**
     returns the first child entity that is a tag
    */
    inline xml_dom_entity *first_child_tag(){
        return first_child( XML_DOM_TAG );
    }
    
    /**
     returns the first child entity that is an attribute
     */
    inline xml_dom_entity *first_child_attribute(){
        return first_child( XML_DOM_ATTRIBUTE );
    }

    /**
     returns the first child entity that is a comment
     */
    inline xml_dom_entity *first_child_comment(){
        return first_child( XML_DOM_COMMENT );
    }

    /**
     returns the first child element of this entity whose type
     matches 'type' and name matches 'name'
    */
    inline xml_dom_entity *first_child( xml_dom_entity_type type, std::string name ){
        if( num_children() > 0 ){
            xml_dom_entity *child = first_child();
            while( child ){
                if( child->m_type == type && child->m_name.compare(name) == 0 )
                    return child;
                child = next_child( child );
            }
            return NULL;
        }
        return NULL;
    }
    
    /**
     returns the first child entity that is a tag
     */
    inline xml_dom_entity *first_child_tag( std::string name ){
        return first_child( XML_DOM_TAG, name );
    }
    
    /**
     returns the first child entity that is an attribute
     */
    inline xml_dom_entity *first_child_attribute( std::string name ){
        return first_child( XML_DOM_ATTRIBUTE, name );
    }
    
    /**
     returns the first child entity that is a comment
     */
    inline xml_dom_entity *first_child_comment( std::string name ){
        return first_child( XML_DOM_COMMENT, name );
    }
    
    /**
     returns the previous child to the entity 'child' of this
     element, which may be a tag, comment or attribute
    */
    inline xml_dom_entity *previous_child( xml_dom_entity *child ){
        assert( child->m_parent == this );
        if( child->m_index >= 0 && child->m_index < num_children()-1 )
            return get_child( child->m_index-1 );
        return NULL;
    }
    
    /**
     returns the previous child to the entity child of this
     element, whose type matches type
    */
    inline xml_dom_entity *previous_child( xml_dom_entity *child, xml_dom_entity_type type ){
        assert( child->m_parent == this );
        child = previous_child( child );
        while( child ){
            if( child->m_type == type )
                return child;
            child = previous_child( child );
        }
        return NULL;
    }
    
    /**
     returns the previous child to the entity child of this
     element, whose type matches type and name matches 'name'
     */
    inline xml_dom_entity *previous_child( xml_dom_entity *child, xml_dom_entity_type type, std::string name ){
        assert( child->m_parent == this );
        child = previous_child( child );
        while( child ){
            if( child->m_type == type && child->m_name.compare(name) == 0 )
                return child;
            child = previous_child( child );
        }
        return NULL;
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_child_tag( xml_dom_entity *child ){
        return previous_child( child, XML_DOM_TAG );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *previous_child_tag( xml_dom_entity *child, std::string name ){
        return previous_child( child, XML_DOM_TAG, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_child_attribute( xml_dom_entity *child ){
        return previous_child( child, XML_DOM_ATTRIBUTE );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *previous_child_attribute( xml_dom_entity *child, std::string name ){
        return previous_child( child, XML_DOM_ATTRIBUTE, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_child_comment( xml_dom_entity *child ){
        return previous_child( child, XML_DOM_COMMENT );
    }
    
    /**
     returns the next child of the entity 'child' of this
     element, which may be a tag, comment or attribute
     */
    inline xml_dom_entity *next_child( xml_dom_entity *child ){
        assert( child->m_parent == this );
        if( child->m_index >= 0 && child->m_index < num_children()-1 )
            return get_child( child->m_index+1 );
        return NULL;
    }
    
    /**
     returns the next child to the entity child of this
     element, whose type matches type
     */
    inline xml_dom_entity *next_child( xml_dom_entity *child, xml_dom_entity_type type ){
        assert( child->m_parent == this );
        child = next_child( child );
        while( child ){
            if( child->m_type == type )
                return child;
            child = next_child( child );
        }
        return NULL;
    }
    
    /**
     returns the previous child to the entity child of this
     element, whose type matches type and name matches 'name'
     */
    inline xml_dom_entity *next_child( xml_dom_entity *child, xml_dom_entity_type type, std::string name ){
        assert( child->m_parent == this );
        child = next_child( child );
        while( child ){
            if( child->m_type == type && child->m_name.compare(name) == 0 )
                return child;
            child = next_child( child );
        }
        return NULL;
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *next_child_tag( xml_dom_entity *child ){
        return next_child( child, XML_DOM_TAG );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *next_child_tag( xml_dom_entity *child, std::string name ){
        return next_child( child, XML_DOM_TAG, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *next_child_attribute( xml_dom_entity *child ){
        return next_child( child, XML_DOM_ATTRIBUTE );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *next_child_attribute( xml_dom_entity *child, std::string name ){
        return next_child( child, XML_DOM_ATTRIBUTE, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *next_child_comment( xml_dom_entity *child ){
        return next_child( child, XML_DOM_COMMENT );
    }

    /**
     convenience method to get the next sibling element. Just
     wraps the get_child() methods.
     */
    inline xml_dom_entity *previous_sibling(){
        if( !m_parent ) return NULL;
        return m_parent->previous_child( this );
    }
    
    /**
     convenience method to get the next sibling element whose
     type matches type. Really just wraps the equivalent
     get_child() methods.
     */
    inline xml_dom_entity *previous_sibling( xml_dom_entity_type type ){
        if( !m_parent ) return NULL;
        return m_parent->previous_child( this, type );
    }
    
    /**
     convenience method to get the next sibling element whose
     type matches type. Really just wraps the equivalent
     get_child() methods.
     */
    inline xml_dom_entity *previous_sibling( xml_dom_entity_type type, std::string name ){
        if( !m_parent ) return NULL;
        return m_parent->previous_child( this, type, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_sibling_tag(){
        return previous_sibling( XML_DOM_TAG );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *previous_sibling_tag( std::string name ){
        return previous_sibling( XML_DOM_TAG, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_sibling_attribute(){
        return previous_sibling( XML_DOM_ATTRIBUTE );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *previous_sibling_attribute( std::string name ){
        return previous_sibling( XML_DOM_ATTRIBUTE, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *previous_sibling_comment(){
        return previous_sibling( XML_DOM_COMMENT );
    }

    /**
     convenience method to get the next sibling element. Just
     wraps the get_child() methods.
     */
    inline xml_dom_entity *next_sibling(){
        if( !m_parent ) return NULL;
        return m_parent->next_child( this );
    }
    
    /**
     convenience method to get the next sibling element whose
     type matches type. Really just wraps the equivalent
     get_child() methods.
     */
    inline xml_dom_entity *next_sibling( xml_dom_entity_type type ){
        if( !m_parent ) return NULL;
        return m_parent->next_child( this, type );
    }
    
    /**
     convenience method to get the next sibling element whose
     type matches type and name matches 'name'. Really just 
     wraps the equivalent get_child() methods.
     */
    inline xml_dom_entity *next_sibling( xml_dom_entity_type type, std::string name ){
        if( !m_parent ) return NULL;
        return m_parent->next_child( this, type, name );
    }
    
    /**
        returns the next sibling of the current entity that
        is a tag
    */
    inline xml_dom_entity *next_sibling_tag(){
        return next_sibling( XML_DOM_TAG );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *next_sibling_tag( std::string name ){
        return next_sibling( XML_DOM_TAG, name );
    }

    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *next_sibling_attribute(){
        return next_sibling( XML_DOM_ATTRIBUTE );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag with name 'name'
     */
    inline xml_dom_entity *next_sibling_attribute( std::string name ){
        return next_sibling( XML_DOM_ATTRIBUTE, name );
    }
    
    /**
     returns the next sibling of the current entity that
     is a tag
     */
    inline xml_dom_entity *next_sibling_comment(){
        return next_sibling( XML_DOM_COMMENT );
    }
    
    /** 
     debugging method for printing
    */
    inline void print( int scope=0 ){
        for( int i=0; i<scope; i++ ){
            printf("  ");
        }
        switch( m_type ){
            case XML_DOM_INVALID:
                break;
            case XML_DOM_DOCUMENT:
                printf("DOCUMENT\n");
                break;
            case XML_DOM_TAG:
                printf("TAG: %s\n", get_name().c_str() );
                break;
            case XML_DOM_COMMENT:
                printf("COMMENT: %s\n", get_value().c_str() );
                break;
            case XML_DOM_ATTRIBUTE:
                printf("ATTRIBUTE: %s=%s\n", get_name().c_str(), get_value().c_str() );
                break;
        }
        
        xml_dom_entity *child = first_child();
        while( child ){
            child->print( scope+1 );
            child = child->next_sibling();
        }
    }
};

/**
 Overload of the << operator to allow DOM's to be streamed into files.
 This can be done from any tag within the file
*/
/*
std::ostream& operator<<( std::ostream &output, xml_dom_entity &item ){
    // dumping an attribute to a file this was would result in malformed xml
    // or at least not what is desired, so catch this error
    if( item.get_type() == XML_DOM_ATTRIBUTE ){
        xml_error("Cannot stream attributes from DOM into file directly.\n");
        return output;
    }
    
    // for the valid types of elements, write the xml to the stream
    switch( item.get_type() ){
        case XML_DOM_TAG:{
            output << "<" << item.get_name();
            xml_dom_entity *attr = item.first_child_attribute();
            while( attr ){
                output << " " << attr->get_name() << "=\"" << attr->get_value() << "\"";
                attr = attr->next_sibling_attribute();
            }
            if( item.first_child_tag() == NULL && item.get_value().size() == 0 )
                output << " />" << std::endl;
            else 
                output << ">" << item.get_value() << std::endl;
        } break;
        case XML_DOM_COMMENT:
            output << "<!-- " << item.get_value() <<  "-->" << std::endl;
            break;
        case XML_DOM_DOCUMENT:
            output << "<?xml version=\"1.0\"?>" << std::endl;
            break;
        case XML_DOM_ATTRIBUTE:
        case XML_DOM_INVALID:
            break;
    }
    
    // print out the child elements
    xml_dom_entity *child = item.first_child();
    while( child ){
        if( child->get_type() != XML_DOM_ATTRIBUTE )
            output << *child;
        child = child->next_sibling();
    }
    
    // close the tag if necessary
    if( item.get_type() == XML_DOM_TAG && (item.first_child_tag() != NULL || item.get_value().size() > 0) ){
        output << "</" << item.get_name() << ">" << std::endl;
        return output;
    }
    // never reached?
    return output;
}
*/
 
/**
 DOM-builder callback for when the xml parser encounters an
 opening tag.  Creates the tag, adds it to the last tag 
 currently on the stack and then pushes the new tag onto the
 stack
*/
static inline void xml_dom_begin_tag_cb( void *user_data, std::string &name ){
    std::list<xml_dom_entity*> *stack = (std::list<xml_dom_entity*>*) user_data;    
    xml_dom_entity *tag = new xml_dom_entity();
    tag->set_type( XML_DOM_TAG );
    tag->set_name( name );
    stack->back()->add_child( tag );
    stack->push_back( tag );
}

/**
 DOM-builder callback for when the parser encounters a
 closing tag. Just pops the tag-stack
*/
static inline void xml_dom_end_tag_cb( void *user_data, std::string &name ){
    std::list<xml_dom_entity*> *stack = (std::list<xml_dom_entity*>*) user_data;
    name=name;
    stack->pop_back();
}

/**
 DOM-builder callback for when the parser encounters the
 text for a tag, simply sets the text of the top tag on
 the tag-stack
*/
static void xml_dom_tag_text_cb( void *user_data, std::string &text ){
    std::list<xml_dom_entity*> *stack = (std::list<xml_dom_entity*>*) user_data;
    stack->back()->set_value( text );
}

/**
 DOM-builder callback for when a comment is encountered
 by the parser.  Just adds the comment to the top tag on
 the tag-stack
*/
static void xml_dom_comment_cb( void *user_data, std::string &comment ){
    std::list<xml_dom_entity*> *stack = (std::list<xml_dom_entity*>*) user_data;
    xml_dom_entity *text = new xml_dom_entity();
    text->set_type( XML_DOM_COMMENT );
    text->set_value( comment );
    stack->back()->add_child( text );
}

/**
 DOM-builder callback for when an attribute is encountered
 by the parser. Adds the attribute to the top tag on the
 tag stack
*/
static void xml_dom_attribute_cb( void *user_data, std::string &name, std::string &value ){
    std::list<xml_dom_entity*> *stack = (std::list<xml_dom_entity*>*) user_data;
    xml_dom_entity *attrib = new xml_dom_entity();
    attrib->set_type( XML_DOM_ATTRIBUTE );
    attrib->set_name( name );
    attrib->set_value( value );
    stack->back()->add_child( attrib );
}

/**
 Parses the document in buffer and returns a pointer to the
 root entity (an xml_dom_entity with type XML_DOM_DOCUMENT)
*/
static xml_dom_entity *xml_dom_parse( std::string &buffer ){
    std::list<xml_dom_entity*> stack;
    
    // create the callback structure and the xml parser state
    xml_callbacks callbacks = { &stack, xml_dom_begin_tag_cb, xml_dom_end_tag_cb, xml_dom_tag_text_cb, xml_dom_comment_cb, xml_dom_attribute_cb };
    xml_state state = { buffer, 0, 0, 0, &callbacks };
    
    // create the root element and push it onto the stack
    xml_dom_entity *doc = new xml_dom_entity();
    doc->set_type( XML_DOM_DOCUMENT );
    stack.push_back( doc );
    
    // parse the document
    xml_read_document( &state );
    
    // return the front of the stack
    return stack.front();
}

#endif
