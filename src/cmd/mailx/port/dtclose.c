#include	"dthdr.h"

/*	Close a dictionary
**
**	Written by Kiem-Phong Vo (07/15/95)
*/
#if __STD_C
int dtclose(reg Dt_t* dt)
#else
int dtclose(dt)
reg Dt_t*	dt;
#endif
{
	if(dt->nview > 0 ) /* can't close if being viewed */
		return -1;

	if(dt->view)	/* turn off viewing */
		dtview(dt,NIL(Dt_t*));

	/* announce the close event */
	if(dt->disc->eventf &&
	   (*dt->disc->eventf)(dt,DT_CLOSE,NIL(Void_t*),dt->disc) < 0)
		return -1;

	/* release all allocated data */
	(void)(*(dt->meth->searchf))(dt,NIL(Void_t*),DT_CLEAR);
	if(dtsize(dt) > 0)
		return -1;

	if(dt->data->ntab > 0)
		(*dt->memoryf)(dt,(Void_t*)dt->data->htab,0,dt->disc);
	(*dt->memoryf)(dt,(Void_t*)dt->data,0,dt->disc);

	free((Void_t*)dt);

	return 0;
}
