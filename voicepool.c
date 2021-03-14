/* Markidis 2019 (c) BSD3 licence */

#include "m_pd.h"

#define DEFNOVOICE (10)

static t_class *voicepool_class;

typedef double t_double;

typedef struct _node
{
  t_int n_value;
  t_clock *n_clock;
  t_double n_deltime;
  struct _node *n_next;
  struct _nodepool *n_nodepool;
} t_node;

typedef struct _nodepool
{
  t_node **np_pool;
  t_node *np_head;
  t_outlet *np_outlet;
  t_int np_noalive;
} t_nodepool;

typedef struct _voicepool
{
  t_object x_obj;
  t_nodepool *x_nodepool;
  t_int    x_novoice;
  t_outlet *x_float_outlet;
  t_outlet *x_noalive_outlet;
} t_voicepool;

static void voicepool_clear(t_voicepool *x)
{
  t_nodepool *np = x->x_nodepool;
  t_int i = 0, novoice = x->x_novoice;
  t_node **tmp = np->np_pool;
  
  for(i=0; i<novoice; i++)
    clock_unset(tmp[i]->n_clock);

  np->np_noalive = 0;
  np->np_head = (*tmp);
  
  outlet_float(x->x_noalive_outlet, np->np_noalive);
}

static void voicepool_post(t_voicepool *x)
{
  t_node *n = x->x_nodepool->np_head;
  while(n != NULL) {
    post("%d", n->n_value);
    n = n->n_next;
  }
}

static void node_tick(t_node *n)
{
  t_nodepool *np = n->n_nodepool;
  n->n_next = np->np_head;
  np->np_head = n;
  outlet_float(np->np_outlet,--np->np_noalive);
}

static t_node *create_node(t_nodepool *ndpool, t_node **nd, t_int v)
{
  t_int i = 0;
  t_node *tmp;
  t_node *p = (*nd);
  tmp = (t_node *)getbytes(sizeof(t_node));
  tmp->n_value = v;
  tmp->n_nodepool = ndpool;
  tmp->n_next = NULL;
  tmp->n_clock = clock_new(tmp,(t_method)node_tick);

  if(p == NULL)
    p = tmp;
  else {
    while(p->n_next != NULL)
      p = p->n_next;
    p->n_next = tmp;
  }
  
  return tmp;
}

static t_node **create_node_pool(t_nodepool *nd, t_int n)
{
  t_int i;
  t_node **tmp;
  tmp = (t_node **)getbytes(n*sizeof(t_node *));
  (*tmp) = NULL;
  /* qui ho allocato lo spazio per n strutture nodi  */
  for(i=0;i<n;i++)
    tmp[i] = create_node(nd, tmp,i);
  
  return tmp;
}

static t_nodepool *create_nodepool(t_voicepool *x, t_int n)
{
  t_nodepool *tmp;
  tmp = (t_nodepool *)getbytes(sizeof(t_nodepool));
  tmp->np_pool = create_node_pool(tmp, n);

  tmp->np_head = (*tmp->np_pool);
  tmp->np_noalive = 0;
  
  return tmp;
}

static void voicepool_ft1(t_voicepool *x, t_float deltime)
{
  t_nodepool *np = x->x_nodepool;
  t_node *head = np->np_head;
  if(head != NULL) {
    outlet_float(x->x_float_outlet,head->n_value);
    clock_delay(head->n_clock,(t_double)deltime);
    x->x_nodepool->np_head = x->x_nodepool->np_head->n_next;
    outlet_float(x->x_noalive_outlet,++np->np_noalive);
  }
  else
    post("pool empty");
}

static void voicepool_free(t_voicepool *x)
{
  t_int i = 0, n = x->x_novoice;

  for(i=0;i<n;i++) {
    clock_free(x->x_nodepool->np_pool[i]->n_clock);
    freebytes(x->x_nodepool->np_pool[i],sizeof(t_node*));
  }
  freebytes(x->x_nodepool->np_pool,sizeof(t_node**));
  freebytes(x->x_nodepool,sizeof(t_nodepool*));

  outlet_free(x->x_float_outlet); // fix 210314
  outlet_free(x->x_no_alive_outlet);
  
}

static void *voicepool_new(t_floatarg f)
{
  t_voicepool *x = (t_voicepool *)pd_new(voicepool_class);
  t_int i = 0, n;
  x->x_novoice = n = f > 0 ? (t_int)f : DEFNOVOICE;

  x->x_nodepool = create_nodepool(x, n);

  x->x_float_outlet = outlet_new(&x->x_obj, 0);
  x->x_noalive_outlet = outlet_new(&x->x_obj, 0);
  x->x_nodepool->np_outlet = x->x_noalive_outlet;
  
  return (x);
}

void voicepool_setup(void)
{
#if PD_FLOATSIZE == 32
  voicepool_class = class_new(gensym("voicepool"), (t_newmethod)voicepool_new,
			      (t_method)voicepool_free,sizeof(t_voicepool),0,
			      A_DEFFLOAT,0);
#elif PD_FLOATSIZE == 64
  voicepool_class = class_new64(gensym("voicepool"), (t_newmethod)voicepool_new,
				(t_method)voicepool_free,sizeof(t_voicepool),0,
				A_DEFFLOAT,0);
#else
#error [voicepool]: invalid FLOATSIZE: must be 32 or 64
#endif
  class_addmethod(voicepool_class,(t_method)voicepool_post,gensym("post"),0);
  class_addmethod(voicepool_class,(t_method)voicepool_clear,gensym("clear"),0);
  class_addfloat(voicepool_class,voicepool_ft1);
  class_sethelpsymbol(voicepool_class,gensym("voicepool-help"));
}
