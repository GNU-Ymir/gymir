##### Global #######

%program := (public | private | version_g | extern | declaration)*
%public := 'public' ( '{'  (declaration | version_g | extern)* '}' ) | (extern | declaration)
%private := 'private' ( '{' (declaration | version_g | extern)* '}' ) | (extern | declaration)
%decl_block := '{' (declaration | public | private | version | extern)* '}'
%version_g := Identifiant decl_block ('else' decl_block)?

%declaration := function 
                | macro 
                | use
                | module 
                | import
                | struct 
                | union
                | enum
                | global 
                | global_imut
                | self 
                | dest_self

####### Macro ####### 

%macro := '__macro' Identifiant '{' (macro_expr block)* '}'
%macro_expr := '(' (macro_var | macro_repeat | macro_token)* ')'
%macro_expr_repeat := (macro_var | macro_repeat | macro_token)* 
%macro_repeat := ('*' | '+') Identifiant ':' '(' %macro_expr_repeat (',' macro_token)? ')'
%macro_var := '$' Identifiant ':' ('ident' | 'expr' | 'block' | 'token')
%macro_token := string 

###### Module ######

%module := ('mod' Identifiant ('.' Identifiant)* ('.' '_')? ';')
            | ('mod' Identifiant (templates_decl)? decl_block)

%self := 'self' '(' ')' block
%dest_self := '~self' '(' ')' block

%global := 'static' Identifiant (('=' expression) | (':' type)) ';'
%global_imut := 'cte' Identifiant '=' expression ';'
%global_extern := (from)? Identifiant ':' type ';'

%space := Identifiant ('.' Identifiant)*
%use := 'use' expression ';'
%import := 'import' (Identifiant ('.' Identifiant)* ('.' '_')?) (',' Identifiant ('.' Identifiant)* ('.' '_')?)* ';'

%of_var := ('const | 'ref')? Identifiant (':' ('...' | left_op_simple) | 'of' left_op_simple | '...')
%templates_decl := '(' of_var (',' of_var)* ')'
%struct := 'struct' ('@' attributes)? (templates_decl)? ('|' struct_var_declaration)* '->' Identifiant ';'

%enum := 'enum' (':' type) ? ('|' Identifiant ':' type)* '->' Identifiant ';'
%attributes := ('{' Identifiant (',' Identifiant)* '}') | Identifiant

###### Function #######

%if_function := 'if' (expression | '(' expression ')')
%function := 'def' ('@' attributes)? Identifiant ((if_function templates_fn_decl) | (templates_fn_ decl)) '(' (fn_var_declaration (',' fn_var_declaration)*)? ')' ('->' ('mut' | 'const' | 'ref')? expression)? ('{' contract '}' | block)

%contract := 'pre' block 'body' block 'post' '(' Identifiant ')' block

%proto := (from)? Identifiant '(' (fn_var_declaration (',' fn_var_declaration)*)? (',' ...)? ')' ('->' ('mut' | 'const' | 'ref')? expression)? ';'

%from := '(' ('C' | ('C++' (',' space)?) | ('D', (',' space)?)) ')'
%extern := 'extern' (proto | global_extern | extern_block)
%extern_block := '{' (function | 'extern' proto | 'extern' global_extern)* '}'

%var := type

%fn_var_declaration := ('const' | 'ref')? Identifiant (':' left_op_simple)?
%struct_var_declaration := Identifiant ':' left_op_simple
%typed_var_declaration := ('const' | 'ref')? Identifiant ':' left_op_simple
%deco_type := ('const' | 'ref' | 'mut') ('(' left_op_simple ')' | '!(' left_op_simple ')' | left_op_simple)

%type := ('[' left_op_simple (';' numeric_or_var)? ']') 
         | deco_type
         | Identifiant ('!' left_op_simple | '!(' left_op_simple* ')')


%Identifiant := ('_')* ([a-z]|[A-Z]) ([a-z]|[A-Z]|'_'|[0-9])*
%block := ('{' (instruction | import | struct | extern | union)* '}') | instruction
%instruction := if 
                | return
                | for 
                | while
                | let 
                | let_destruct
                | break
                | assert
                | pragma
                | scope
                | version
                | cte
                | match
                | expression_ult ';'
                | ';' 

%cte := 'cte' (if | for | assert)
%version := 'version' Identifiant block ('else' block)?
%let := 'let' ('cte' | 'const' | static)? Identifiant ('=' expression)? (',' ('cte' | 'const' | static)? Identifiant ('=' expression)?)* ';'
%let_destruct := 'let' '(' Identifiant (',' Identifiant)* ')' '=' expression ';'

%expression_ult := expression (Ult_op expression_ult)?
%expression := ulow ((Exp_op expression)?
%ulow := low (Ulow_op ulow)?
%low := high (Low_op low)?
%high := pth (High_op high)?
%pth := (Bef_unary expression) | pth_par | block | pth_wpar
%pth_par := (lambda_expr | tuple_expr | expression)
%pth_wpar := literal | left_op (Af_unary)?
%left_op := cast | const_array | func_ptr | mixin | match | pragma 

%lambda_expr := '(' (fn_var_declaration (',' fn_var_declaration)*)? ')' ('=>' expression) | block
%tuple_expr := '('  ((expression (',' expression)+) | (expression ','))? ')'

%literal := int | float | string | bool | expand | is | typeof | Ignore | stringof | lambda_move | null
%literal_simple := int | float | string | bool | expand | is | typeof | stringof  | null

%typeof := 'typeof' '(' ('mut')? expression ')'
%stringof := 'stringof' '(' expression ')'
%is := 'is' '(' expression ':' (expression | 'fn' | 'tuple' | 'struct' | 'dg') ')'

%numeric_or_var := int | var
%string := '\"' [.]* '\"'


%Ult_op := '/=' | '&=' | '|=' | '-=' | '+=' | '>>=' | '<<=' | '=' | '*=' | '%=' | '^=' | '^^=' | '~='
%Exp_op := '||' | '&&'
%Ulow_op := '<' | '>' | '<=' | '>=' | '!=' | '!<' | '!<=' | '!>' | '!>=' | '==' | '..' | '!is' | 'is'
%Low_op := '+' | '|' | '>>' | '^' | '~' | '-' | '<<'
%High_op := '/' | '&' | '*' | '%' | '^^'
%Suite_elem := '(' | '[' | '.' | '::' | '{' | ':'
%Af_unary := '++' | '--'
%Bef_unary := '-' | '&' | '*' | '!'
