#include "9cc.h"
#include <stdlib.h>

// 2項演算子のNodeを作成
static Node	*new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

// 単項演算子(整数値)のNodeを作成
static Node	*new_node_number(int val)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return (node);
}

static Node	*stmt(void);
static Node	*expr(void);
static Node	*assign(void);
static Node	*equality(void);
static Node	*relational(void);
static Node	*add(void);
static Node	*mul(void);
static Node	*unary(void);
static Node	*primary(void);

void	program(void)
{
	int	i = 0;

	while (!at_eof() && i < 100)
		code[i++] = stmt();
	code[i] = NULL;
}

static Node	*stmt(void)
{
	Node	*node;

	if (consume("return"))
	{
		node = new_node(ND_RETURN, expr(), NULL);
		expect(";");
		return (node);
	}

	if (consume("if"))
	{
		expect("(");
		node = new_node(ND_IF, NULL, NULL);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		if (consume("else"))
			node->els = stmt();
		return (node);
	}

	if (consume("while"))
	{
		expect("(");
		node = new_node(ND_WHILE, NULL, NULL);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		return (node);
	}

	if (consume("for"))
	{
		expect("(");
		node = new_node(ND_FOR, NULL, NULL);
		if (!consume(";"))
		{
			node->lhs = expr();
			expect(";");
		}
		if (!consume(";"))
		{
			node->cond = expr();
			expect(";");
		}
		if (!consume(")"))
		{
			node->rhs = expr();
			expect(")");
		}
		node->then = stmt();
		return (node);
	}

	node = expr();
	expect(";");
	return (node);
}

static Node	*expr(void)
{
	return (assign());
}

static Node	*assign(void)
{
	Node	*node = equality();

	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return (node);
}

static Node	*equality(void)
{
	Node	*node = relational();

	while (true)
	{
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQ, node, relational());
		else
			return (node);
	}
}

static Node	*relational(void)
{
	Node	*node = add();

	while (true)
	{
		if (consume("<="))
			node = new_node(ND_LTE, node, add());
		else if (consume(">="))
			node = new_node(ND_GTE, node, add());
		else if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume(">"))
			node = new_node(ND_GT, node, add());
		else
			return (node);
	}
}

static Node	*add(void)
{
	Node	*node = mul();

	while (true)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return (node);
	}
}

static Node	*mul(void)
{
	Node	*node = unary();

	while (true)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return (node);
	}
}

static Node	*unary(void)
{
	if (consume("+"))
		return (unary());
	else if (consume("-"))
		return (new_node(ND_SUB, new_node_number(0), unary()));
	return primary();
}

static Node	*primary(void)
{
	if (consume("("))
	{
		Node	*node = expr();
		consume(")");
		return (node);
	}

	Token	*tok = consume_ident();
	if (tok)
	{
		Node	*node = (Node *)calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar	*lvar = find_lvar(tok);
		if (lvar)
			node->offset = lvar->offset;
		else
		{
			lvar = (LVar *)calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			if (locals)
				lvar->offset = locals->offset + 8;
			else
				lvar->offset = 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return (node);
	}

	return (new_node_number(expect_number()));
}
