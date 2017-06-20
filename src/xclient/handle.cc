/*
 * This is free software; see the source for copying conditions.  There is NO
 * warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "handle.h"

extern bool is_register;
extern uint32_t apply_id0;
extern uint32_t apply_id1;

int handle(std::list<Transport*> *w, std::map<int, Transport*> *m, std::map<uint32_t, int> *__m, Transport* t) {
  int ret = 0;
  int i = 0;
  uint32_t length = 0;
  uint32_t id[2] = {0, 0};

  t->pr();
  do {
    /* MESSAGE HEAD */
    if (t->get_rp() >= 3 * sizeof (uint32_t)) {
      memcpy(&length, t->get_rx(), sizeof (uint32_t));
      length = ntohl(length);

      memcpy(&id[0], (char *)t->get_rx() + sizeof (uint32_t), sizeof (uint32_t));
      id[0] = ntohl(id[0]);

      memcpy(&id[1], (char *)t->get_rx() + 2 * sizeof (uint32_t), sizeof (uint32_t));
      id[1] = ntohl(id[1]);

      plog(notice, "[+] Transaction(%d) Begin: length = 0x%x, id = {0x%x(0x%x), 0x%x}\n", i, length, id[0], t->get_id(), id[1]);
    } else {
      /* Back to wait message. */
      break;
    }

    /* MESSAGE BODY */
    if (id[0] == id[1] && id[0] != 0 && t->get_rp() >= (3 * sizeof (uint32_t) + length)) {
      if (t->get_id() != id[0]) {
        if (t->get_id() != 0) {
          __m->erase(t->get_id());
          plog(warning, "Erase pair{id(0x%x), fd(%d)} from __m(%p)\n", t->get_id(), t->get_fd(), __m);
        }

        t->set_id(id[0]);
        __m->insert(std::make_pair(t->get_id(), t->get_fd()));
        plog(info, "Insert pair{id(0x%x), fd(%d)} into __m(%p)\n", t->get_id(), t->get_fd(), __m);
        plog(notice, "Register respond. id = 0x%x\n", id[0]);

        is_register = true;
      }

//      t->set_wx(t->get_rx(), 3 * sizeof (uint32_t) + length);
      memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
      t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
//      w->push_back(t);
    } else if (t->get_rp() >= (3 * sizeof (uint32_t) + length)) {
      plog(notice, "Message completed(=0x%x).\n", (unsigned int)(3 * sizeof (uint32_t)) + length);
      if (t->get_id() != id[1]) {
        plog(warning, "id = 0x%x, It's NOT me(0x%x)!\n", id[0], t->get_id());
        t->clear_rx();
        break;
      }

      int fx = 0;
      Transport* tx = NULL;

      std::map<uint32_t, int>::iterator ie = __m->find(id[1]);
      if (ie != __m->end()) {
        fx = ie->second;
      } else {
        plog(info, "Back to wait id(0x%x)\n", id[1]);
        break;
      }

      std::map<int, Transport*>::iterator im = m->find(fx);
      if (im != m->end()) {
        tx = im->second;
      } else {
        plog(info, "Back to wait id(0x%x)\n", id[1]);
        break;
      }

//      tx->set_wx(t->get_rx(), (3 * sizeof (uint32_t) + length));
      memmove(t->get_rx(), (const void *)((char *)t->get_rx() + (3 * sizeof (uint32_t) + length)), t->get_rp() - (3 * sizeof (uint32_t) + length));
      t->set_rp(t->get_rp() - (3 * sizeof (uint32_t) + length));
//      w->push_back(tx);
      plog(debug, "w = %p\n", w);
    } else {
      /* Back to wait message. */
      break;
    }

    plog(info, "[x] Transaction(%d) Passed.\n", i++);
  } while (true);

  if (t->get_rp() != 0) {
    plog(info, "[!] Transaction(%d) Cancel! length = 0x%x, t->rp = 0x%lx\n", i, length, t->get_rp());
  }
  return ret;
}

int serializing(Transport *t) {
  int ret = 0;
  assert(t != NULL);

  void *message = 0;
  uint32_t length = 0;

  length = sizeof (uint32_t) * 3 + CONTENT_MAX;
  message = malloc(length);
  memset(message, 0, length);

  uint32_t id0 = htonl(apply_id0);
  uint32_t id1 = htonl(apply_id1);
  memcpy((void *) ((char *)message + sizeof (uint32_t)), &id0, sizeof (uint32_t)); // message id0

  char content[CONTENT_MAX] = {0};
  char str[DATE_MAX];
  sysdate(str);
  char string0[] = {"abcdefghijklmnopqrstuvwxyz"};

  string0[rand() % strlen(string0)] = 0x0;
  snprintf(content, sizeof content, "sysdate = %s, random = %s", str, strfry(string0));
  size_t content_length = strlen(content);

  do {
    if (is_register) {
      if (id0 == id1) { break; }
      memcpy((void *) ((char *)message + sizeof (uint32_t) * 2), &id1, sizeof (uint32_t)); // message id1
      plog(notice, "Generate normal request: |0x%lx|0x%x|0x%x|%s|\n", content_length, apply_id0, apply_id1, content);
    } else {
      memcpy((void *) ((char *)message + sizeof (uint32_t) * 2), &id0, sizeof (uint32_t)); // message id1
      plog(notice, "Generate register request: |0x%lx|0x%x|0x%x|%s|\n", content_length, apply_id0, apply_id0, content);
    }
  } while (0);

  memcpy((void *) ((char *)message + sizeof (uint32_t) * 3), &content, content_length); // message content
  uint32_t length0 = htonl(content_length);
  memcpy(message, &length0, sizeof (uint32_t));

  t->set_wx(message, sizeof (uint32_t) * 3 + content_length);

  free(message);
  message = NULL;

  return ret;
}

