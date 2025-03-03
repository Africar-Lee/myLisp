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
    char *name;
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

lval *lval_new(void);
lval *lval_num(double x);
lval *lval_err(char *fmt, ...);
lval *lval_sym(char *s);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
lval *lval_func(lbuiltin func);
lval *lval_lambda(lval *formals, lval *body);

lval *lval_add_tail(lval *v, lval *x);
lval *lval_add_head(lval *v, lval *x);
lval *lval_set_name(lval *v, char *name);
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
