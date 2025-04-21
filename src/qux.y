%token END 0
%token RETURN "return" FOR "for" IF "if" ELSE "else" IDENTIFIER NUMLITERAL STRINGLITERAL
%token OR "||" AND "&&" EQ "==" NE "!=" PP "++" MM "--" 
%left ','
%left "||"
%left "&&"
%left "==" "!="
%left '+' '-'
%left '*' '/' '%'
%left '(' '['
%%

library:   declerations;
declerations: declerations decleration
|          %empty;
decleration: IDENTIFIER ':' ':' rvalue ';'
rvalue: function;
function: '(' paramteres ')' stmnt;
stmnt: stmnts
     | decleration ';'
     | "if" expr stmnt
     | "for" expr stmnt
     | "return" expr ';';
stmnts: '{' stmnts1 '}';
stmnts1: stmnt stmnts1
       | %empty;
paramteres: paramteres ',' parameter
          | %empty;
parameter: IDENTIFIER;
expr: NUMLITERAL
    | STRINGLITERAL
    | IDENTIFIER
    | '(' expr ')'
    | IDENTIFIER '(' ')'
    | expr '+' expr
    | expr '-' expr %prec '+'
    | expr '/' expr
    | expr '*' expr %prec '/'
    | expr ',' expr;
%%
