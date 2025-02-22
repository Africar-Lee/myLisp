#include <stdio.h>
#include "mpc.h"

#ifdef _WIN32
/* Declare a buffer for user of size 2048 */
static char buffer[2048];

void add_history(char *unused) {}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

//=============================================================
//             Declaration
//=============================================================

#define LASSERT(args, cond, fmt, ...)             \
    if (!(cond))                                  \
    {                                             \
        lval *err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args);                           \
        return err;                               \
    }

#define LASSERT_TYPE(func, args, index, expect)                     \
    LASSERT(args, args->cell[index]->type == expect,                \
            "Function '%s' passed incorrect type for argument %i. " \
            "Got %s, Expected %s.",                                 \
            func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num)                               \
    LASSERT(args, args->count == num,                              \
            "Function '%s' passed incorrect number of arguments. " \
            "Got %i, Expected %i.",                                \
            func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index)     \
    LASSERT(args, args->cell[index]->count != 0, \
            "Function '%s' passed {} for argument %i.", func, index);

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Create Enumeration of Possible lval Types */
typedef enum LVAL_TYPE
{
    LVAL_ERR = 0,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_FUNC,
    LVAL_SEXPR,
    LVAL_QEXPR,
} LVAL_TYPE;

/* Create Enumeration of Error types */
typedef enum LERR_TYPE
{
    DIV_BY_ZERO = 0,
    POW_ON_NEG,
    OP_ON_NAN,
    STR_TO_NUM,
    BAD_OP,
    SEXPR_NO_FUNC,
    MOD_ON_FLT_AND_OVFLW,
    HEAD_TAIL_TOO_MANY_ARGS,
    HEAD_TAIL_BAD_TYPE,
    HEAD_TAIL_EMPTY,
    LERR_TYPE_NUM,
} LERR_TYPE;

/* Create Array of Error strings */
char *LERR_STR[LERR_TYPE_NUM] = {
    [DIV_BY_ZERO] = "Division by zero!",
    [POW_ON_NEG] = "Pow base on negtive number!",
    [OP_ON_NAN] = "Cannot operate on non-number!",
    [STR_TO_NUM] = "This String cannot cast to number!",
    [BAD_OP] = "This operation has not been support!",
    [SEXPR_NO_FUNC] = "First element is not a function!",
    [MOD_ON_FLT_AND_OVFLW] = "Numbers in mod-op shouldn't be float type!\n\
Overflow occurred in type cast!",
    [HEAD_TAIL_TOO_MANY_ARGS] = "Function 'head/tail' passed too many arguments!",
    [HEAD_TAIL_BAD_TYPE] = "Function 'head/tail' passed incorrect types!",
    [HEAD_TAIL_EMPTY] = "Function 'head/tail' passed {}!",
};

typedef lval *(*lbuiltin)(lenv *, lval *);

/* Declare New lval Struct */
typedef struct lval
{
    int type;
    double num;

    /* Error and symbol types have string data */
    char *err;
    char *sym;

    /* Function */
    lbuiltin builtin;
    lenv *env;
    lval *formals;
    lval *body;

    /* Count and Point to a list of "lval*" */
    int count;
    struct lval **cell;
} lval;

struct lenv
{
    lenv *par;
    int count;
    char **syms;
    lval **vals;
};

char *ltype_name(int t);

void test_exit(lval *v, int *p_flag);

lenv *lenv_new(void);
void lenv_del(lenv *env);
lenv *lenv_copy(lenv *e);
lval *lenv_get_value(lenv *e, lval *k);
lval *lenv_get_key(lenv *e, lbuiltin v);
void lenv_def(lenv *e, lval *k, lval *v);
void lenv_put(lenv *e, lval *k, lval *v);
void lenv_add_builtin(lenv *e, char *name, lbuiltin func);
void lenv_add_builtins(lenv *e);

lval *lval_num(double x);
lval *lval_err(char *fmt, ...);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_func(lbuiltin func);
lval *lval_lambda(lval *formals, lval *body);

lval *lval_add_tail(lval *v, lval *x);
lval *lval_add_head(lval *v, lval *x);
lval *lval_copy(lval *v);
void lval_del(lval *v);
lval *lval_take(lval *v, int i);
lval *lval_pop(lval *v, int i);
lval *lval_join(lval *x, lval *y);

lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_eval(lenv *e, lval *v);

lval *lval_call(lenv *e, lval *f, lval *a);

// lval *builtin(lenv *e, lval *v, char *func);
lval *builtin_exit(lenv *e, lval *a);
lval *builtin_penv(lenv *e, lval *a);
lval *builtin_def(lenv *e, lval *a);
lval *builtin_put(lenv *e, lval *a);
lval *builtin_var(lenv *e, lval *a, char *func);
lval *builtin_lambda(lenv *e, lval *a);

lval *builtin_op(lenv *e, lval *v, char *op);
lval *builtin_add(lenv *e, lval *a);
lval *builtin_sub(lenv *e, lval *a);
lval *builtin_mul(lenv *e, lval *a);
lval *builtin_div(lenv *e, lval *a);

lval *builtin_head(lenv *e, lval *v);
lval *builtin_tail(lenv *e, lval *v);
lval *builtin_list(lenv *e, lval *v);
lval *builtin_eval(lenv *e, lval *v);
lval *builtin_join(lenv *e, lval *v);
lval *builtin_cons(lenv *e, lval *v);
lval *builtin_len(lenv *e, lval *v);
lval *builtin_init(lenv *e, lval *v);

void lval_expr_print(lenv *e, lval *v, char open, char close);
void lval_print(lenv *e, lval *v);

//=======================================================
//                Implemention
//=======================================================

char *ltype_name(int t)
{
    switch (t)
    {
    case LVAL_FUNC:
        return "Function";
    case LVAL_NUM:
        return "Number";
    case LVAL_ERR:
        return "Error";
    case LVAL_SYM:
        return "Symbol";
    case LVAL_SEXPR:
        return "S-Expression";
    case LVAL_QEXPR:
        return "Q-Expression";
    default:
        return "Unknown";
    }
}

void test_exit(lval *val, int *p_flag)
{
    if (val->type == LVAL_FUNC && (strcmp(val->sym, "exit") == 0))
        *p_flag = 0;
}

/****************
 * Constructors
 ****************/
lenv *lenv_new(void)
{
    lenv *e = (lenv *)malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

/* Construct a pointer to a new Number lval */
lval *lval_num(double x)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */
lval *lval_err(char *fmt, ...)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_ERR;

    /* Create a va list and initialize it */
    va_list va;
    va_start(va, fmt);

    /* Allocate 512 Bytes of space */
    v->err = malloc(512);

    /* Printf the error string with a maximum of 511 characters */
    vsnprintf(v->err, 511, fmt, va);

    /* Reallocate to number of bytes actually used */
    v->err = realloc(v->err, strlen(v->err) + 1);

    /* Clean up our va list */
    va_end(va);

    return v;
}

/* Construct a pointer to a new Symbol lval */
lval *lval_sym(char *s)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/* Construct a pointer to a new empty Sexpr lval */
lval *lval_sexpr(void)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* Construct a pointer to a new empty Qexpr lval */
lval *lval_qexpr(void)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* Construct a pointer to a new empty Built-In Func lval */
lval *lval_func(lbuiltin func)
{
    lval *v = (lval *)malloc(sizeof(lval));
    v->type = LVAL_FUNC;
    v->builtin = func;
    return v;
}

lval *lval_lambda(lval *formals, lval *body)
{
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_FUNC;

    /* Set builtin to NULL */
    v->builtin = NULL;

    /* Build new environment */
    v->env = lenv_new();

    /* Set Formals and Body */
    v->formals = formals;
    v->body = body;

    return v;
}

/**************
 * Destructor
 **************/

/* Delete a lval */
void lval_del(lval *v)
{
    if (!v)
        return;

    switch (v->type)
    {
    /* Do nothing special for number type */
    case LVAL_NUM:
        break;

    /* For Err or Sym free the string data */
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->sym);
        break;

    case LVAL_FUNC:
        if (!v->builtin)
        {
            lenv_del(v->env);
            lval_del(v->formals);
            lval_del(v->body);
        }
        break;

    /* If Sexpr then delete all elements inside */
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        for (int i = 0; i < v->count; i++)
        {
            lval_del(v->cell[i]);
        }
        /* Also free the memory allocated to contain the pointers */
        free(v->cell);
        break;
    }

    /* Free the memory allocated to the lval v itself */
    free(v);

    return;
}

void lenv_del(lenv *env)
{
    for (int i = 0; i < env->count; ++i)
    {
        free(env->syms);
        lval_del(env->vals[i]);
    }
    free(env->syms);
    free(env->vals);
    free(env);
}

/**************
 *  Modifier
 **************/

lenv *lenv_copy(lenv *e)
{
    lenv *n = malloc(sizeof(lenv));

    n->par = e->par;
    n->count = e->count;
    n->syms = malloc(sizeof(char *) * e->count);
    n->vals = malloc(sizeof(lval *) * e->count);
    for (int i = 0; i < e->count; ++i)
    {
        n->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }

    return n;
}

/* Using key to get lval from environment */
lval *lenv_get_value(lenv *e, lval *k)
{
    /* Iterate overall items in env */
    for (int i = 0; i < e->count; ++i)
    {
        /* Check if stored string matches the symbol string */
        if (strcmp(e->syms[i], k->sym) == 0)
            return lval_copy(e->vals[i]);
    }
    /* If no symbol, check in parent env otherwise error */
    if (e->par)
    {
        return lenv_get_value(e->par, k);
    }
    return lval_err("Unbound symbol: %s!", k->sym);
}

/* Using function pointer to get its name from environment */
lval *lenv_get_key(lenv *e, lbuiltin f)
{
    lval *res = NULL;
    /* Iterate overall items in env */
    for (int i = 0; i < e->count; ++i)
    {
        /* Check if stored pointer matches the func pointer */
        if (e->vals[i]->builtin == f)
        {
            res = lval_copy(e->vals[i]);
            res->sym = malloc(strlen(e->syms[i]) + 1);
            strcpy(res->sym, e->syms[i]);
            return res;
        }
    }

    /* If no val check in parent env otherwise error */
    if (e->par)
    {
        return lenv_get_key(e->par, f);
    }
    return lval_err("No such function in environment!");
}

/**
 * @brief Try to copy a k-v pair into environment
 *
 * @param e The specific environment
 * @param k lval of symbol, key
 * @param v lval of contents, value
 */
void lenv_put(lenv *e, lval *k, lval *v)
{
    /* Iterate overall items in env */
    for (int i = 0; i < e->count; ++i)
    {
        /* Check if stored string matches the symbol string */
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    /* If no symbol, allocate new space for it */
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    /* Copy contents of lval and symbol string into new location */
    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}

/* Define value in the outmost env */
void lenv_def(lenv *e, lval *k, lval *v)
{
    /* Find the outmost env */
    while (e->par)
        e = e->par;

    /* Put value in */
    lenv_put(e, k, v);
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func)
{
    lval *k = lval_sym(name);
    lval *v = lval_func(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
    return;
}

void lenv_add_builtins(lenv *e)
{
    /* List Functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "init", builtin_init);

    /* Mathematical Functions */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);

    /* Variable Functions */
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);

    /* Lambda Functions */
    lenv_add_builtin(e, "\\", builtin_lambda);

    /* Exit Function */
    lenv_add_builtin(e, "exit", builtin_exit);

    /* Print Functions */
    lenv_add_builtin(e, "penv", builtin_penv);

    return;
}

lval *lval_read_num(mpc_ast_t *t)
{
    errno = 0;
    double x = strtod(t->contents, NULL);
    return errno != ERANGE
               ? lval_num(x)
               : lval_err(LERR_STR[STR_TO_NUM]);
}

lval *lval_read(mpc_ast_t *t)
{
    /* If Symbol or Number then return conversion to that type */
    if (strstr(t->tag, "number"))
        return lval_read_num(t);
    if (strstr(t->tag, "symbol"))
        return lval_sym(t->contents);

    /* If root (>) or sexpr or qexpr then create empty list */
    lval *x = NULL;
    if (strcmp(t->tag, ">") == 0)
        x = lval_sexpr();
    if (strstr(t->tag, "sexpr"))
        x = lval_sexpr();
    if (strstr(t->tag, "qexpr"))
        x = lval_qexpr();

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++)
    {
        if (strcmp(t->children[i]->contents, "(") == 0)
            continue;
        if (strcmp(t->children[i]->contents, ")") == 0)
            continue;
        if (strcmp(t->children[i]->contents, "{") == 0)
            continue;
        if (strcmp(t->children[i]->contents, "}") == 0)
            continue;
        if (strcmp(t->children[i]->tag, "regex") == 0)
            continue;
        x = lval_add_tail(x, lval_read(t->children[i]));
    }

    return x;
}

lval *lval_add_tail(lval *v, lval *x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval *lval_add_head(lval *v, lval *x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    lval **temp_array = malloc(sizeof(lval *) * (v->count - 1));
    memmove(temp_array, v->cell, sizeof(lval *) * (v->count - 1));
    memcpy(&(v->cell[1]), temp_array, sizeof(lval *) * (v->count - 1));
    v->cell[0] = x;

    free(temp_array);
    return v;
}

lval *lval_copy(lval *v)
{
    lval *x = (lval *)malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type)
    {
    /* Copy Functions and Numbers Directly */
    case LVAL_FUNC:
        if (x->builtin)
        {
            x->builtin = v->builtin;
        }
        else
        {
            x->builtin = NULL;
            x->env = lenv_copy(v->env);
            x->formals = lval_copy(v->formals);
            x->body = lval_copy(v->body);
        }
        break;
    case LVAL_NUM:
        x->num = v->num;
        break;
    /* Copy Strings use malloc and strcpy */
    case LVAL_ERR:
        x->err = (char *)malloc(strlen(v->err) + 1);
        strcpy(x->err, v->err);
        break;
    case LVAL_SYM:
        x->sym = (char *)malloc(strlen(v->sym) + 1);
        strcpy(x->sym, v->sym);
        break;
    /* Copy lists by coping each sub-expressions */
    case LVAL_QEXPR:
    case LVAL_SEXPR:
        x->count = v->count;
        x->cell = (lval **)malloc(sizeof(lval) * v->count);
        for (int i = 0; i < x->count; ++i)
            x->cell[i] = lval_copy(v->cell[i]);
        break;

    default:
        break;
    }
    return x;
}

lval *lval_set_sym(lval *v, char *sym)
{
    LASSERT(v, v->type != LVAL_ERR || v->type != LVAL_SYM,
            "func-%s, line-%d: Invalid type", __func__, __LINE__);

    v->sym = malloc(strlen(sym) + 1);
    strcpy(v->sym, sym);

    return v;
}

/* Print a "lval" */
void lval_print(lenv *e, lval *v)
{
    switch (v->type)
    {
    /* In the case the type is a number print it, then 'break' out of the switch. */
    case LVAL_NUM:
        printf("%g", v->num);
        break;
    case LVAL_ERR:
        printf("Error: %s", v->err);
        break;
    case LVAL_SYM:
        printf("%s", v->sym);
        break;
    case LVAL_FUNC:
        if (v->builtin)
        {
            printf("<builtin>: %s", v->sym);
        }
        else
        {
            printf("(\\ ");
            lval_print(e, v->formals);
            putchar(' ');
            lval_print(e, v->body);
            putchar(')');
        }

        if (strcmp(v->sym, "penv") == 0)
        {
            printf("\n    <name>  --    <type>\n");
            for (int i = 0; i < e->count; ++i)
            {
                printf("%10s  --  %10s\n", e->syms[i], ltype_name(e->vals[i]->type));
            }
            printf("total: %d\n", e->count);
        }
        break;
    case LVAL_SEXPR:
        lval_expr_print(e, v, '(', ')');
        break;
    case LVAL_QEXPR:
        lval_expr_print(e, v, '{', '}');
        break;
    }

    return;
}

/* Print an "lval" followed by a newline */
void lval_println(lenv *e, lval *v)
{
    lval_print(e, v);
    putchar('\n');
}

void lval_expr_print(lenv *e, lval *v, char open, char close)
{
    putchar(open);
    for (int i = 0; i < v->count; ++i)
    {
        /* Print Value contained within */
        lval_print(e, v->cell[i]);

        /* Do not print trailing space if last element */
        if (i != (v->count - 1))
            putchar(' ');
    }

    putchar(close);
}

char *readline(char *prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

lval *lval_eval_sexpr(lenv *e, lval *v)
{
    /* Evaluate Children */
    for (int i = 0; i < v->count; ++i)
        v->cell[i] = lval_eval(e, v->cell[i]);

    /* Error Checking */
    for (int i = 0; i < v->count; ++i)
        if (v->cell[i]->type == LVAL_ERR)
            return lval_take(v, i);

    /* Empty Expression */
    if (v->count == 0)
        return v;
    /* Single Expression */
    if (v->count == 1)
        return lval_take(v, 0);

    /* Ensure First Element is Function after evaluation */
    lval *f = lval_pop(v, 0);
    if (f->type != LVAL_FUNC)
    {
        lval_del(f);
        lval_del(v);
        return lval_err(LERR_STR[SEXPR_NO_FUNC]);
    }

    /* Call function to get result */
    lval *result = lval_call(e, f, v);
    lval *symbol = lenv_get_key(e, f->builtin);
    if (symbol->type == LVAL_SYM)
    {
        result->sym = malloc(strlen(symbol->sym) + 1);
        memset(result->sym, 0, strlen(symbol->sym));
        strcpy(result->sym, symbol->sym);
    }
    else
    {
        result->sym = malloc(512);
        char *temp = "not-builtin\0";
        strcpy(result->sym, temp);
        result->sym = realloc(result->sym, strlen(temp) + 1);
    }

    lval_del(symbol);
    lval_del(f);
    return result;
}

lval *lval_eval(lenv *e, lval *v)
{
    if (v->type == LVAL_SYM)
    {
        lval *x = lenv_get_value(e, v);
        x = lval_set_sym(x, v->sym);

        lval_del(v);
        return x;
    }
    /* Evaluate Sexpressions */
    if (v->type == LVAL_SEXPR)
        return lval_eval_sexpr(e, v);

    /* All Other lval types remain the same */
    return v;
}

lval *lval_call(lenv *e, lval *f, lval *a)
{
    /* If builtin then simply call that */
    if (f->builtin)
        return f->builtin(e, a);

    /* Record Argument Counts */
    int given = a->count;
    int total = f->formals->count;

    /* While arguments still remain to be processed */
    while (a->count)
    {
        /* If we have run out of formal arguments to bind */
        if (f->formals->count == 0)
        {
            lval_del(a);
            return lval_err("Function passed too many arguments. "
                            "Got %i, Expected %i.",
                            given, total);
        }

        /* Bind corresponding formals and args into the func's env */
        lval *sym = lval_pop(f->formals, 0);
        lval *val = lval_pop(a, 0);
        lenv_put(f->env, sym, val);

        lval_del(sym);
        lval_del(val);
    }

    /* Arguments have been all bound, so delete the arg list */
    lval_del(a);

    /* If all formals have been bound evaluate */
    if ((f->formals->count == 0))
    {
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(f->env, lval_add_tail(lval_sexpr(), lval_copy(f->body)));
    }
    else
    {
        /* Otherwise return partially evaluated func */
        return lval_copy(f);
    }
}

/* Pop out the first child of list v */
lval *lval_pop(lval *v, int i)
{
    /* Find the item[i] */
    lval *x = v->cell[i];

    /* Move the children remained */
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

    /* Decrease the count of items in the list */
    v->count--;

    /* Reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);

    return x;
}

/* Take the ith child of lval v, and then delete the original list */
lval *lval_take(lval *v, int i)
{
    lval *x = lval_pop(v, i);
    lval_del(v);

    return x;
}

lval *lval_join(lval *x, lval *y)
{
    /* For each cell in 'y', add it to 'x' */
    while (y->count)
    {
        x = lval_add_tail(x, lval_pop(y, 0));
    }

    /* Delete the empty 'y' and return 'x' */
    lval_del(y);
    return x;
}

// lval *builtin(lenv *e, lval *v, char *func)
// {
//     if (strcmp("head", func) == 0)
//         return builtin_head(e, v);
//     if (strcmp("tail", func) == 0)
//         return builtin_tail(e, v);
//     if (strcmp("list", func) == 0)
//         return builtin_list(e, v);
//     if (strcmp("eval", func) == 0)
//         return builtin_eval(e, v);
//     if (strcmp("join", func) == 0)
//         return builtin_join(e, v);
//     if (strcmp("cons", func) == 0)
//         return builtin_cons(e, v);
//     if (strcmp("len", func) == 0)
//         return builtin_len(e, v);
//     if (strcmp("init", func) == 0)
//         return builtin_init(e, v);
//     if (strstr("+-*/%^addsubmuldivmod", func))
//         return builtin_op(e, v, func);
//     lval_del(v);
//     return lval_err("Unkown Function!");
// }

lval *builtin_exit(lenv *e, lval *a)
{
    LASSERT(a, a->count == 1, "Function 'exit' has no argument!");

    /* Delete lval a */
    lval_del(a);

    /* Delete the environment */
    for (int i = 0; i < e->count; ++i)
    {
        lval_del(e->vals[i]);
        free(e->syms[i]);
    }

    return lval_sym("exit");
}

lval *builtin_penv(lenv *e, lval *a)
{
    LASSERT(a, a->count == 1, "Function 'penv' has no argument!");
    lval_del(a);
    return lval_sym("penv");
}

/**
 * @brief Define global functions
 *
 * @param e Environment
 * @param a A lval lsits function symbols and arguments
 * @return A empty S-expr on success
 */
lval *builtin_def(lenv *e, lval *a)
{
    return builtin_var(e, a, "def");
}

lval *builtin_put(lenv *e, lval *a)
{
    return builtin_var(e, a, "=");
}

lval *builtin_lambda(lenv *e, lval *a)
{
    /* Check two arguments, each of which are Q-Expressions */
    LASSERT_NUM("\\", a, 2)
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

    /* Check first Qexpr contains only symbols */
    for (int i = 0; i < a->cell[0]->count; ++i)
    {
        LASSERT(a, a->cell[0]->cell[i]->type == LVAL_SYM,
                "Cannot define non-symbol. Got %s, Expected %s.",
                ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    /* Pop first two arguments and pass them to lval_lambda */
    lval *formals = lval_pop(a, 0);
    lval *body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval *builtin_var(lenv *e, lval *a, char *func)
{
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval *syms = a->cell[0];
    for (int i = 0; i < syms->count; i++)
    {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
                "Function '%s' cannot define non-symbol. "
                "Got %s, Expected %s.",
                func,
                ltype_name(syms->cell[i]->type),
                ltype_name(LVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count - 1),
            "Function '%s' passed too many arguments for symbols. "
            "Got %i, Expected %i.",
            func, syms->count, a->count - 1);

    for (int i = 0; i < syms->count; ++i)
    {
        /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0)
        {
            lenv_def(e, syms->cell[i], a->cell[i + 1]);
        }
        if (strcmp(func, "=") == 0)
        {
            lenv_put(e, syms->cell[i], a->cell[i + 1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

lval *builtin_op(lenv *e, lval *v, char *op)
{
    /* Ensure all arguments are numbers */
    for (int i = 0; i < v->count; ++i)
    {
        if (v->cell[i]->type != LVAL_NUM)
        {
            lval_del(v);
            return lval_err(LERR_STR[OP_ON_NAN]);
        }
    }

    /* Pop the first element */
    lval *x = lval_pop(v, 0);

    /* If no arguments and sub then perform unary negation */
    if ((strcmp(op, "-") == 0) && v->count == 0)
    {
        x->num = -x->num;
    }

    /* While there are still arguments remaining */
    while (v->count > 0)
    {
        /* Pop the next element */
        lval *y = lval_pop(v, 0);

        if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0)
            x->num += y->num;
        if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0)
            x->num -= y->num;
        if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0)
            x->num *= y->num;
        if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0)
        {
            if (y->num == 0)
            {
                lval_del(x);
                lval_del(y);
                x = lval_err(LERR_STR[DIV_BY_ZERO]);
                break;
            }
            x->num /= y->num;
        }
        if (strcmp(op, "%") == 0)
        {
            errno = 0;
            int temp_x = (int)x->num;
            int temp_y = (int)y->num;
            if (errno == ERANGE)
            {
                lval_del(x);
                lval_del(y);
                x = lval_err(LERR_STR[MOD_ON_FLT_AND_OVFLW]);
                break;
            }
            x->num = temp_x % temp_y;
        }
        if (strcmp(op, "^") == 0)
        {
            if (x->num < 0)
            {
                lval_del(x);
                lval_del(y);
                x = lval_err(LERR_STR[POW_ON_NEG]);
                break;
            }
            else if (x->num == 0 && y->num == 0)
                x->num = 1;
            else
                x->num = pow(x->num, y->num);
        }

        lval_del(y);
    }

    lval_del(v);
    return x;
}

lval *builtin_add(lenv *e, lval *a)
{
    return builtin_op(e, a, "+");
}

lval *builtin_sub(lenv *e, lval *a)
{
    return builtin_op(e, a, "-");
}

lval *builtin_mul(lenv *e, lval *a)
{
    return builtin_op(e, a, "*");
}

lval *builtin_div(lenv *e, lval *a)
{
    return builtin_op(e, a, "/");
}

lval *builtin_head(lenv *e, lval *v)
{
    /* Check Errors */
    LASSERT(v, v->count == 1, LERR_STR[HEAD_TAIL_TOO_MANY_ARGS]);
    LASSERT(v, v->cell[0]->type == LVAL_QEXPR, LERR_STR[HEAD_TAIL_BAD_TYPE]);
    LASSERT(v, v->cell[0]->count != 0, LERR_STR[HEAD_TAIL_EMPTY]);

    /* Otherwise take the first argument */
    lval *x = lval_take(v, 0);

    /* Delete all elements that are not head and return */
    while (x->count > 1)
        lval_del(lval_pop(x, 1));

    return x;
}

lval *builtin_tail(lenv *e, lval *v)
{
    /* Check Errors */
    LASSERT(v, v->count == 1, LERR_STR[HEAD_TAIL_TOO_MANY_ARGS]);
    LASSERT(v, v->cell[0]->type == LVAL_QEXPR, LERR_STR[HEAD_TAIL_BAD_TYPE]);
    LASSERT(v, v->cell[0]->count != 0, LERR_STR[HEAD_TAIL_EMPTY]);

    /* Take first argument */
    lval *x = lval_take(v, 0);

    /* Delete first element and return */
    lval_del(lval_pop(x, 0));

    return x;
}

lval *builtin_list(lenv *e, lval *v)
{
    v->type = LVAL_QEXPR;
    return v;
}

lval *builtin_eval(lenv *e, lval *v)
{
    LASSERT(v, v->count == 1, "Function 'eval' passed too many arguments!");
    LASSERT(v, v->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type!");

    lval *x = lval_take(v, 0);
    x->type = LVAL_SEXPR;

    return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *v)
{
    for (int i = 0; i < v->count; ++i)
    {
        LASSERT(v,
                v->cell[i]->type == LVAL_QEXPR,
                "Function 'join' passed incorrect type.");
    }

    lval *x = lval_pop(v, 0);

    while (v->count)
    {
        x = lval_join(x, lval_pop(v, 0));
    }
    lval_del(v);

    return x;
}

lval *builtin_cons(lenv *e, lval *v)
{
    LASSERT(v, v->count == 2, "Function 'cons' passed wrong number of args.");
    LASSERT(v, v->cell[1]->type == LVAL_QEXPR, "Function 'cons' passed incorrect type!");

    lval *x = lval_pop(v, 0);
    lval *y = lval_take(v, 0);
    y = lval_add_head(y, x);

    return y;
}

lval *builtin_len(lenv *e, lval *v)
{
    LASSERT(v, v->cell[0]->type == LVAL_QEXPR, "Function 'len' passed incorrect type!");
    lval *x = lval_num(v->cell[0]->count);
    lval_del(v);

    return x;
}

lval *builtin_init(lenv *e, lval *v)
{
    LASSERT(v, v->count == 1, LERR_STR[HEAD_TAIL_TOO_MANY_ARGS]);
    LASSERT(v, v->cell[0]->type == LVAL_QEXPR, LERR_STR[HEAD_TAIL_BAD_TYPE]);
    LASSERT(v, v->cell[0]->count != 0, LERR_STR[HEAD_TAIL_EMPTY]);

    lval *x = lval_pop(v->cell[0], v->cell[0]->count - 1);
    lval_del(x);

    return v->cell[0];
}

int main(int argc, char **argv)
{
    /* Create some parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    /* Define them with following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
              "\
                number      : /-?[0-9]+([.][0-9]*)?/;\
                symbol      : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ | '%' | '^';\
                sexpr       : '(' <expr>* ')';\
                qexpr       : '{' <expr>* '}';\
                expr        : <number> | <symbol> | <sexpr> | <qexpr>;\
                lispy       : /^/ <expr>* /$/;\
              ",
              Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    lenv *env = lenv_new();
    lenv_add_builtins(env);

    /* Print Version and Exit Information */
    puts("Lispy Version 0.0.6");
    puts("Press Ctrl+c to Exit\n");

    int running = 1;
    /* In a never ending loop */
    while (running)
    {
        char *input = readline("lispy> ");
        add_history(input);

        /* Attempt to Parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            /* On Success Print the AST */
            // mpc_ast_print(r.output);
            // lval *result = eval(r.output);
            lval *x = lval_eval(env, lval_read(r.output));
            lval_println(env, x);
            mpc_ast_delete(r.output);
            test_exit(x, &running);
            lval_del(x);
        }
        else
        {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    /* Undefine and Delete our Parsers */
    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}
