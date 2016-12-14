#include "queue.h"

ngx_queue_t ngx_posted_accept_events;
ngx_queue_t ngx_posted_events;

void ngx_event_process_posted_accept(poll_event_t *poll_event, ngx_queue_t *posted)
{
    ngx_queue_t *q;
    poll_event_t *ev;

    while (!ngx_queue_empty(posted)) {
        q = ngx_queue_head(posted);
        ev = ngx_queue_data(q, poll_event_t, queue);

        ngx_queue_remove(&(ev)->queue);
        ev->accept_cb(poll_event, );
    }
}

void ngx_event_process_posted_read(poll_event_t *poll_event, ngx_queue_t *posted)
{
    ngx_queue_t *q;
    poll_event_t *ev;
    poll_event_element_t *elem;

    while (!ngx_queue_empty(posted)) {
        q = ngx_queue_head(posted);
        ev = ngx_queue_data(q, poll_event_t, queue);
        elem = ev->

        ngx_queue_remove(&(ev)->queue);
        ev->read_cb(poll_event, );
    }
}
