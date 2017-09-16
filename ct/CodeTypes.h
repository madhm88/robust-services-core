//==============================================================================
//
//  CodeTypes.h
//
//  Copyright (C) 2012-2017 Greg Utas.  All rights reserved.
//
#ifndef CODETYPES_H_INCLUDED
#define CODETYPES_H_INCLUDED

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>
#include "SysTypes.h"

using namespace NodeBase;

//------------------------------------------------------------------------------

namespace CodeTools
{
//  C++ keyword strings.
//
extern fixed_string AUTO_STR;
extern fixed_string BOOL_STR;
extern fixed_string BREAK_STR;
extern fixed_string CATCH_STR;
extern fixed_string CASE_STR;
extern fixed_string CHAR_STR;
extern fixed_string CLASS_STR;
extern fixed_string CONST_STR;
extern fixed_string CONST_CAST_STR;
extern fixed_string CONSTEXPR_STR;
extern fixed_string CONTINUE_STR;
extern fixed_string DEFAULT_STR;
extern fixed_string DELETE_STR;
extern fixed_string DELETE_ARRAY_STR;
extern fixed_string DOUBLE_STR;
extern fixed_string DYNAMIC_CAST_STR;
extern fixed_string DO_STR;
extern fixed_string ELSE_STR;
extern fixed_string ENUM_STR;
extern fixed_string EXPLICIT_STR;
extern fixed_string EXTERN_STR;
extern fixed_string FALSE_STR;
extern fixed_string FLOAT_STR;
extern fixed_string FOR_STR;
extern fixed_string FRIEND_STR;
extern fixed_string IF_STR;
extern fixed_string INLINE_STR;
extern fixed_string INT_STR;
extern fixed_string LONG_STR;
extern fixed_string MUTABLE_STR;
extern fixed_string NAMESPACE_STR;
extern fixed_string NEW_STR;
extern fixed_string NEW_ARRAY_STR;
extern fixed_string NOEXCEPT_STR;
extern fixed_string NULLPTR_STR;
extern fixed_string NULLPTR_T_STR;
extern fixed_string OPERATOR_STR;
extern fixed_string OVERRIDE_STR;
extern fixed_string PRIVATE_STR;
extern fixed_string PROTECTED_STR;
extern fixed_string PUBLIC_STR;
extern fixed_string REINTERPRET_CAST_STR;
extern fixed_string RETURN_STR;
extern fixed_string SHORT_STR;
extern fixed_string SIGNED_STR;
extern fixed_string SIZEOF_STR;
extern fixed_string STATIC_STR;
extern fixed_string STATIC_CAST_STR;
extern fixed_string STRUCT_STR;
extern fixed_string SWITCH_STR;
extern fixed_string TEMPLATE_STR;
extern fixed_string THIS_STR;
extern fixed_string THROW_STR;
extern fixed_string TRUE_STR;
extern fixed_string TRY_STR;
extern fixed_string TYPEDEF_STR;
extern fixed_string TYPEID_STR;
extern fixed_string TYPENAME_STR;
extern fixed_string UNION_STR;
extern fixed_string UNSIGNED_STR;
extern fixed_string USING_STR;
extern fixed_string VIRTUAL_STR;
extern fixed_string VOID_STR;
extern fixed_string WHILE_STR;

extern fixed_string DEFINED_STR;
extern fixed_string HASH_DEFINE_STR;
extern fixed_string HASH_ELIF_STR;
extern fixed_string HASH_ELSE_STR;
extern fixed_string HASH_ENDIF_STR;
extern fixed_string HASH_ERROR_STR;
extern fixed_string HASH_IF_STR;
extern fixed_string HASH_IFDEF_STR;
extern fixed_string HASH_IFNDEF_STR;
extern fixed_string HASH_INCLUDE_STR;
extern fixed_string HASH_LINE_STR;
extern fixed_string HASH_PRAGMA_STR;
extern fixed_string HASH_UNDEF_STR;

//------------------------------------------------------------------------------
//
//  Other parser strings.
//
extern fixed_string ARRAY_STR;
extern fixed_string COMMENT_END_STR;
extern fixed_string COMMENT_START_STR;
extern fixed_string COMMENT_STR;
extern fixed_string ELLIPSES_STR;
extern fixed_string LOCALS_STR;

//------------------------------------------------------------------------------
//
//  Valid initial characters in an identifier.  '#' and '~' are included so that
//  preprocessor directives and destructor names can be treated as keywords and
//  identifiers, respectively.
//
extern const std::string ValidFirstChars;

//  Valid subsequent characters in an identifier.
//
extern const std::string ValidNextChars;

//  Valid subsequent characters in a template specification.
//
extern const std::string ValidTemplateSpecChars;

//  Valid characters in an operator.
//
extern const std::string ValidOpChars;

//  Valid characters in an integer literal.
//
extern const std::string ValidIntChars;

//  Valid digits in an integer, hex, and octal literal, respectively.
//
extern const std::string ValidIntDigits;
extern const std::string ValidHexDigits;
extern const std::string ValidOctDigits;

//------------------------------------------------------------------------------
//
//  A file's role in declaring and defining an item.
//
struct FileRole
{
   bool isDeclarer;  // declares item
   bool isDefiner;   // defines item
};

//------------------------------------------------------------------------------
//
//  For adding and removing levels of pointer indirection and for counting
//  arrays and references.
//
typedef int8_t TagCount;

//------------------------------------------------------------------------------
//
//  Types of functions.
//
enum FunctionType
{
   FuncCtor,
   FuncDtor,
   FuncOperator,
   FuncStandard
};

//------------------------------------------------------------------------------
//
//  Roles of functions.
//
enum FunctionRole
{
   FuncOther,  // none of those below
   PureCtor,   // constructor for a new object
   CopyCtor,   // copy constructor
   MoveCtor,   // move constructor
   PureDtor,   // destructor
   CopyOper,   // copy (assignment) operator
   MoveOper    // move (assignment) operator
};

//------------------------------------------------------------------------------
//
//  How and where a function that can be defaulted or deleted is defined.
//
enum FunctionDefinition
{
   NotDeclared,    // not declared in this class or a base class
   LocalDeclared,  // declared by class (implemented, defaulted, or deleted)
   BaseDefined,    // defined by base class (implemented or defaulted)
   BaseDeleted     // deleted by base class
};

//------------------------------------------------------------------------------
//
//  Controls whether a function is "executed" (to produce pseudo object code)
//  and, for a header, indicates whether it contains function templates or an
//  entire class template.
//
enum TemplateLocation
{
   FuncNoTemplate,  // no template involvement
   FuncIsTemplate,  // a function template
   FuncInTemplate   // a function in a class template
};

//------------------------------------------------------------------------------
//
//  The distance between a class and subclass or a scope and subscope.
//  NOT_A_SUBSCOPE and NOT_A_SUBCLASS indicate that the distance is
//  "infinite".
//
typedef uint16_t Distance;

constexpr Distance NOT_A_SUBSCOPE = UINT16_MAX;
constexpr Distance NOT_A_SUBCLASS = UINT16_MAX;

//------------------------------------------------------------------------------
//
//  The accessibility of a symbol, based on the scope that declares it and
//  the scope that uses it.  Whether the symbol is visible (through #include
//  and using statements, and scope qualification) is considered separately.
//
enum Accessibility
{
   Declared,      // user is declarer
   Inherited,     // user is a subclass of declarer and can see name
   Unrestricted,  // defined in a .h* or user is a friend of declarer
   Restricted,    // defined in a .c*
   Inaccessible   // none of the above apply (e.g. private)
};

//------------------------------------------------------------------------------
//
//  How a symbol was made visible by a using statement.
//
enum UsingMode
{
   NoUsing,       // not accessed through a using statement
   ScopeUsing,    // using statement within scope (class or function)
   FileUsing,     // using statement in same file
   IncludedUsing  // using statement in another file
};

//------------------------------------------------------------------------------
//
//  Indicates how closely a type matches the one expected by a function or
//  template.
//
enum TypeMatch
{
   Incompatible,   // argument cannot be matched
   Adaptable,      // argument is non-const but would be passed as const
   Abridgeable,    // argument could be truncated (e.g. int to char)
   Convertible,    // argument must be converted (e.g. enum to int)
   Constructible,  // argument must be passed to a constructor
   Promotable,     // argument can be promoted (numeric to one of greater range)
   Compatible      // argument matches without modification
};

//------------------------------------------------------------------------------
//
//  Information about a symbol's accessibility within a specific scope.
//
struct SymbolView
{
   //  The symbol's accessibility.
   //
   Accessibility accessibility : 4;

   //  If and how the symbol was resolved by a using statement.
   //
   UsingMode mode : 4;

   //  How well the symbol's arguments matched those supplied.
   //
   TypeMatch match : 4;

   //  Set if the symbol was accessible because of a friend declaration.
   //
   bool Friend : 2;

   //  Set if the symbol has a referent.  True except for unresolved
   //  forward and friend declarations.
   //
   bool resolved : 2;

   //  If ACCESSIBILITY is Inherited, the distance (in the class hierarchy)
   //  from the class that defined the symbol to the class that used it.
   //  Otherwise, the distance from the scope that defined the symbol to
   //  the scope that used it.
   //
   Distance distance : 16;
};

//  For initializing SymbolView instances.
//
constexpr SymbolView NotAccessible =
   {Inaccessible, NoUsing, Compatible, false, true, 0};
constexpr SymbolView DeclaredGlobally =
   {Unrestricted, NoUsing, Compatible, false, true, 0};
constexpr SymbolView DeclaredLocally =
   {Declared, NoUsing, Compatible, false, true, 0};

//------------------------------------------------------------------------------
//
//  Specifies a type's role in a template.
//
enum TemplateRole
{
   TemplateNone,       // not part of a template
   TemplateArgument,   // e.g. int in vector< int >
   TemplateParameter,  // e.g. T in template< typename T > class vector{...};
   TemplateClass       // a DataSpec created interally for template matching;
                       // contains each of the parameters to a class template
};

//------------------------------------------------------------------------------
//
//  Types of assignments.
//
enum AssignmentType
{
   Copied,   // right-hand side of an assignment operator
   Passed,   // passed as an argument
   Returned  // returned as a function result
};

//------------------------------------------------------------------------------
//
//  Source code warnings.
//
enum Warning
{
   DefineNotAtFileScope,     // #define appears within a class or function
   IncludeNotAtFileScope,    // #include appears within a class or function
   UseOfSlashAsterisk,       // use of /* */ comment
   UseOfNull,                // use of NULL
   PtrTagDetached,           // <type> *<data> instead of <type>* data
   RefTagDetached,           // <type> &<data> instead of <type>& data
   UseOfCast,                // use of C-style cast: (type) expr
   FunctionalCast,           // type(expr) is equivalent to a C-style cast
   ReinterpretCast,          // use of reinterpret cast
   Downcasting,              // use of cast down inheritance hierarchy
   CastingAwayConstness,     // use of cast to remove const qualifier
   PointerArithmetic,        // use of pointer arithmetic
   RemoveSemicolon,          // unnecessary semicolon
   RedundantConst,           // more than one const qualifier for same token
   IncludeGuardMissing,      // no #include guard
   IncludeNotSorted,         // #include not sorted in standard order
   IncludeDuplicated,        // #include already exists for this file
   UsingInHeader,            // header contains using directive or declaration
   UsingDuplicated,          // using statement duplicated
   ArgumentUnused,           // argument not used
   ClassUnused,              // no members used
   DataUnused,               // data is neither read nor written
   EnumUnused,               // enum not referenced
   EnumeratorUnused,         // enumerator not referenced
   ForwardUnused,            // forward declaration did not resolve a symbol
   FriendUnused,             // friend did not access a restricted member
   FunctionUnused,           // function not invoked
   TypedefUnused,            // typedef not referenced
   UsingUnused,              // using statement did not resolve a symbol
   ForwardUnresolved,        // no referent found for forward declaration
   FriendUnresolved,         // no referent found for friend declaration
   FriendAsForward,          // friend declaration is also forward declaration
   HidesInheritedName,       // member has the same name as a base class member
   ClassCouldBeNamespace,    // only enums, typedefs, or static functions used
   ClassCouldBeStruct,       // no subclasses and no non-static function invoked
   RedundantAccessControl,   // previous member already public/protected/private
   ItemCouldBePrivate,       // item only used within declarer
   ItemCouldBeProtected,     // item only used within declarer and subclasses
   PointerTypedef,           // typedef of pointer type
   AnonymousEnum,            // declaration of unnamed enum
   DataUninitialized,        // global data is not initialized
   DataInitOnly,             // data is initialized but not read or written
   DataWriteOnly,            // data is not read
   GlobalStaticData,         // global static data defined in header
   DataNotPrivate,           // data member is not private
   DataCannotBeConst,        // for detecting const logic errors
   DataCannotBeConstPtr,     // for detecting const logic errors
   DataCouldBeConst,         // data item could be declared as const
   DataCouldBeConstPtr,      // data item could be declared as a const pointer
   DataNeedNotBeMutable,     // no const function modifies this data member
   DefaultPODConstructor,    // use of default constructor; class has POD member
   DefaultConstructor,       // use of default constructor
   DefaultCopyConstructor,   // use of default copy constructor
   DefaultAssignment,        // use of default assignment operator
   PublicConstructor,        // base class has public constructor
   NonExplicitConstructor,   // constructor should be tagged explicit
   MemberInitMissing,        // item missing from member initialization list
   MemberInitNotSorted,      // item missorted in member initialization list
   DefaultDestructor,        // use of default destructor
   VirtualDestructor,        // virtual destructor is not public
   NonVirtualDestructor,     // non-virtual base class destructor not protected
   VirtualFunctionInvoked,   // constructor or destructor calls virtual function
   RuleOf3DtorNoCopyCtor,    // destructor defined, but not copy constructor
   RuleOf3DtorNoCopyOper,    // destructor defined, but not copy operator
   RuleOf3CopyCtorNoOper,    // copy constructor defined, but not copy operator
   RuleOf3CopyOperNoCtor,    // copy operator defined, but not copy constructor
   OperatorOverloaded,       // operator && or || overloaded
   FunctionNotDefined,       // function not implemented
   PureVirtualNotDefined,    // pure virtual function not implemented
   VirtualAndPublic,         // function is both public and virtual
   VirtualOverloading,       // function reuses name of virtual function
   FunctionNotOverridden,    // virtual function has no overrides
   VirtualTagMissing,        // add virtual tag to function declaration
   OverrideTagMissing,       // add override tag to function declaration
   VoidAsArgument,           // use of (void) to specify function parameter
   AnonymousArgument,        // declaration of unnamed argument
   AdjacentArgumentTypes,    // adjacent arguments have the same type
   VirtualDefaultArgument,   // virtual function defines default argument
   ArgumentCannotBeConst,    // for detecting const logic errors
   ArgumentCouldBeConstRef,  // object could be passed by const reference
   ArgumentCouldBeConst,     // argument could be declared as const
   FunctionCannotBeConst,    // for detecting const logic errors
   FunctionCouldBeConst,     // function could be declared as const
   FunctionCouldBeStatic,    // function could be declared as static
   FunctionCouldBeFree,      // function could be declared in namespace
   StaticFunctionViaMember,  // static function invoked via "." or "->" operator
   NonBooleanConditional,    // conditional expression is not a boolean
   EnumTypesDiffer,          // binary operator uses two different enum types
   UseOfTab,                 // tab character in line
   Indentation,              // indentation not a multiple of Indent_Size spaces
   TrailingSpace,            // space after last non-blank character
   AdjacentSpaces,           // extra embedded space in source code
   InsertBlankLine,          // add blank line
   RemoveBlankLine,          // adjacent blank lines
   LineLength,               // line length exceeds 80 characters
   FunctionNotSorted,        // function does not appear in alphabetical order
   HeadingNotStandard,       // lines at top of file do not follow template
   IncludeGuardMisnamed,     // #include guard name is not based on filename
   DebugFtNotInvoked,        // function does not invoke Debug::ft
   DebugFtNotFirst,          // function invokes Debug::ft after first statement
   DebugFtNameMismatch,      // function name string for Debug::ft is incorrect
   DisplayNotOverridden,     // class should override Base.Display
   PatchNotOverridden,       // class should override Object.Patch
   Warning_N                 // number of warnings
};

//  Inserts a string for WARNING into STREAM.
//
std::ostream& operator<<(std::ostream& stream, Warning warning);

//  Returns true if WARNING is associated with an unused item.
//
bool IsUnusedItemWarning(Warning warning);

//------------------------------------------------------------------------------
//
//  Types of source code lines.  Used for line counts.
//
enum LineType
{
   Code,                  // source code
   Blank,                 // blank lines
   EmptyComment,          // //
   SeparatorComment,      // // followed by repeated -, =, or /
   TaggedComment,         // //@ (@ = any character except -, =, or /)
   TextComment,           // //  text
   SlashAsteriskComment,  // /*
   OpenBrace,             // {
   CloseBrace,            // }
   CloseBraceSemicolon,   // };
   DebugFt,               // Debug::ft(Class_Func);
   FunctionName,          // fn_name Class_Func = "Class.Func";
   FunctionNameSplit,     // fn_name Class_Func =\n "Class.Func";
   IncludeDirective,      // #include
   HashDirective,         // #ifndef #define #endif et al
   UsingDirective,        // using
   AnyLine,               // all lines
   LineType_N             // number of line types
};

//  Inserts a string for TYPE into STREAM.
//
std::ostream& operator<<(std::ostream& stream, LineType type);

//------------------------------------------------------------------------------
//
//  Options for the Display function.
//
enum CodeDisplayOptions
{
   DispFQ,    // display fully qualified name
   DispLF,    // insert optional line feed
   DispNoLF,  // omit line feed
   DispLast,  // set for the last item in a series
   DispCode,  // output will be used to generate code
   DispNoAC,  // omit access control prefix
   DispNoTP   // omit template parameters definition list
};

extern const Flags FQ_Mask;
extern const Flags LF_Mask;
extern const Flags NoLF_Mask;
extern const Flags Last_Mask;
extern const Flags Code_Mask;
extern const Flags NoAC_Mask;
extern const Flags NoTP_Mask;

extern uint8_t Indent_Size;

//------------------------------------------------------------------------------
//
//  Returns the index of ITEM in V.  Returns SIZE_MAX if ITEM isn't found.
//
template< typename T > size_t IndexOf(const std::vector< T >& v, const T& item)
{
   for(size_t i = 0; i < v.size(); ++i)
   {
      if(v[i] == item) return i;
   }

   return SIZE_MAX;
}

//------------------------------------------------------------------------------
//
//  Displays the objects in a vector.
//
template< typename T > void DisplayObjects(const std::vector< T >& group,
   std::ostream& stream, const std::string& prefix, const Flags& options)
{
   for(auto i = group.cbegin(); i != group.cend(); ++i)
   {
      (*i)->Display(stream, prefix, options);
   }
}

//------------------------------------------------------------------------------
//
//  Strings used by the Trim and Apply commands.
//
extern fixed_string ADD_INCLUDE_STR;
extern fixed_string REMOVE_INCLUDE_STR;
extern fixed_string ADD_FORWARD_STR;
extern fixed_string REMOVE_FORWARD_STR;
extern fixed_string REMOVE_USING_STR;
}
#endif