// =============================================================================
/*!
 * \file       apps/transport.cpp
 *
 * This file contains the implementation of the a HAN-FUN transport layer over the
 * libuv library.
 *
 * \author     Filipe Alves <filipe.alves@bithium.com>
 *
 * \version    0.3.1
 *
 * \copyright  Copyright &copy; &nbsp; 2014 Bithium S.A.
 */
// =============================================================================
#include <iostream>
#include <iomanip>

#include <cassert>
#include <cstdlib>
#include <cstdint>

#include "uv.h"

#include "hanfun.h"

#include "application.h"

#include "transport.h"

#include "common.h"

// =============================================================================
// Defines
// =============================================================================

#define NONE_MSG    0xFFFF
#define HELLO_MSG   0x0101
#define DATA_MSG    0x0201

#define CHECK_STATUS()                                  \
   if (status != 0)                                     \
   {                                                    \
      print_error (uv_last_error (uv_default_loop ())); \
      exit (-1);                                        \
   }

struct msg_t
{
   uint16_t              nbytes;
   uint16_t              primitive;
   HF::Common::ByteArray data;

   msg_t(uint16_t primitive = NONE_MSG):
      primitive (primitive)
   {}

   msg_t(uint16_t primitive, HF::Common::ByteArray &data):
      primitive (primitive), data (data)
   {}

   size_t size () const
   {
      return sizeof(nbytes) + sizeof(primitive) + data.size ();
   }

   size_t pack (HF::Common::ByteArray &array, size_t offset = 0) const
   {
      size_t start  = offset;

      uint16_t temp = (uint16_t) (sizeof(uint16_t) + data.size ());

      offset += array.write (offset, temp);

      offset += array.write (offset, primitive);

      std::copy (data.begin (), data.end (), array.begin () + offset);

      return offset - start;
   }

   size_t unpack (HF::Common::ByteArray &array, size_t offset = 0)
   {
      size_t start = offset;

      offset += array.read (offset, nbytes);

      offset += array.read (offset, primitive);

      size_t data_size = nbytes - sizeof(primitive);

      data = HF::Common::ByteArray (data_size);

      auto begin = array.begin ();

      begin += offset;

      auto end = begin + data_size;

      std::copy (begin, end, data.begin ());

      return offset - start;
   }
};

struct hello_msg_t
{
   uint8_t      core;
   uint8_t      profiles;
   uint8_t      interfaces;

   HF::UID::UID *uid;

   hello_msg_t():
      core (HF::CORE_VERSION), profiles (HF::PROFILES_VERSION), interfaces (HF::INTERFACES_VERSION)
   {}

   size_t size () const
   {
      return 3 * sizeof(uint8_t) + sizeof(uint8_t) + (uid != nullptr ? uid->size () : 0);
   }

   size_t pack (HF::Common::ByteArray &array, size_t offset = 0) const
   {
      size_t start = offset;

      offset += array.write (offset, core);
      offset += array.write (offset, profiles);
      offset += array.write (offset, interfaces);

      offset += HF::UID::pack (*uid, array, offset);

      return offset - start;
   }

   size_t unpack (HF::Common::ByteArray &array, size_t offset = 0)
   {
      size_t start = offset;

      offset += array.read (offset, core);
      offset += array.read (offset, profiles);
      offset += array.read (offset, interfaces);

      offset += HF::UID::unpack (uid, array, offset);

      return offset - start;
   }
};

// =============================================================================
// Libuv Helpers
// =============================================================================

// =============================================================================
// alloc_buffer
// =============================================================================
/*!
 * This function is used to allocate the buffers used with libuv.
 *
 * @param [in] handle            a pointer to the handle to allocate the buffer for.
 * @param [in] suggested_size    number bytes to allocate.
 *
 * @return  buffer allocated.
 */
// =============================================================================
uv_buf_t alloc_buffer (uv_handle_t *handle, size_t suggested_size)
{
   UNUSED (handle);
   return uv_buf_init ((char *) calloc (1, suggested_size), suggested_size);
}

// =============================================================================
// print_error
// =============================================================================
/*!
 *
 * @param status
 */
// =============================================================================
void print_error (uv_err_t status)
{
   LOG (ERROR) << uv_err_name ((uv_err_t &) status) << " - "
               << uv_strerror ((uv_err_t &) status) << NL;
}

static void handle_message (HF::Transport::Link *link, msg_t &msg);

// =============================================================================
// on_close
// =============================================================================
/*!
 * This function is the callback for when a stream is closed.
 *
 * @param [in] handle   handle to the stream being closed.
 */
// =============================================================================
static void on_close (uv_handle_t *handle)
{
   LOG (DEBUG) << "Connection Closed!" << NL;

   UNUSED (handle);
}

// =============================================================================
// on_read
// =============================================================================
/*!
 * This is the callback for when data has been received in the stream.
 *
 * @param [in] stream   pointer to the stream that received the data.
 * @param [in] nread    number of bytes received.
 * @param [in] buf      buffer containing the data received.
 */
// =============================================================================
static void on_read (uv_stream_t *stream, ssize_t nread, uv_buf_t buf)
{
   HF::Application::Link *link     = (HF::Application::Link *) stream->data;

   HF::Application::Transport *tsp = (HF::Application::Transport *) link->transport ();

   LOG (TRACE) << __PRETTY_FUNCTION__ << " : " << stream << " : " << link << NL;

   if (nread < 0)
   {
      LOG (DEBUG) << "Could not read from stream !" << NL;
      LOG (DEBUG) << "Stream closed !" << NL;

      tsp->remove (link);

      uv_close ((uv_handle_t *) stream, on_close);
   }
   else if (nread > 0)
   {
      HF::Common::ByteArray payload ((uint8_t *) buf.base, nread);

      LOG (TRACE) << "RECV : " << payload << NL;

      msg_t msg;
      msg.unpack (payload);

      LOG (TRACE) << "Size      : " << msg.nbytes << NL;
      LOG (TRACE) << "Primitive : " << std::hex << std::setw (4) << std::setfill ('0')
                  << msg.primitive << NL;
      LOG (TRACE) << "Data      : " << msg.data << NL;

      handle_message (link, msg);
   }

   free (buf.base);
}

// =============================================================================
// on_write
// =============================================================================
/*!
 * This is the callback for when data has been written to a stream.
 *
 * @param [in] req      pointer to the write request that was completed.
 * @param [in] status   error code for the operation.
 */
// =============================================================================
static void on_write (uv_write_t *req, int status)
{
   CHECK_STATUS ();

   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   assert (req->type == UV_WRITE);

   /* Free the read/write buffer and the request */
   free (req);
}

static void send_message (uv_stream_t *stream, msg_t &msg)
{
   HF::Common::ByteArray payload (msg.size ());
   msg.pack (payload);

   LOG (TRACE) << "SEND : " << payload << NL;

   uv_write_t *req = (uv_write_t *) calloc (1, sizeof(uv_write_t));
   uv_buf_t   buf  = uv_buf_init ((char *) payload.data (), payload.size ());

   uv_write (req, (uv_stream_t *) stream, &buf, 1 /*nbufs*/, on_write);
}

static void send_hello (uv_stream_t *stream, HF::Application::Transport *tsp)
{
   hello_msg_t hello;

   hello.uid = const_cast <HF::UID::UID *>(tsp->uid ());

   msg_t msg (HELLO_MSG);

   msg.data = HF::Common::ByteArray (hello.size ());

   hello.pack (msg.data);

   send_message (stream, msg);
}

static void handle_message (HF::Transport::Link *link, msg_t &msg)
{
   HF::Application::Transport *tsp = (HF::Application::Transport *) link->transport ();

   switch (msg.primitive)
   {
      case HELLO_MSG:
      {
         hello_msg_t hello;

         hello.unpack (msg.data);

         LOG (TRACE) << hello.uid << NL;

         ((HF::Application::Link *) link)->uid (hello.uid);

         tsp->add (link);

         break;
      }
      case DATA_MSG:
      {
         tsp->receive (link, msg.data);
         break;
      }
      default:
         break;
   }
}

#ifdef HF_BASE_APP

static void on_connect (uv_stream_t *server, int status)
{
   CHECK_STATUS ();

   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   uv_tcp_t *client = (uv_tcp_t *) calloc (1, sizeof(uv_tcp_t));

   client->data = nullptr;

   uv_tcp_init (server->loop, client);

   if (uv_accept (server, (uv_stream_t *) client))
   {
      LOG (ERROR) << "Could not accept the connection !" << NL;
      uv_close ((uv_handle_t *) client, on_close);
   }
   else
   {
      LOG (INFO) << "Connection accepted !" << NL;

      HF::Application::Transport *tsp = ((HF::Application::Transport *) server->data);
      HF::Application::Link *link     = new HF::Application::Link (tsp, (uv_stream_t *) client);

      client->data = link;

      uv_read_start ((uv_stream_t *) client, (uv_alloc_cb) alloc_buffer, on_read);

      send_hello ((uv_stream_t *) client, tsp);
   }
}

// =============================================================================
// API
// =============================================================================

// =============================================================================
// Transport::initialize
// =============================================================================
/*!
 *
 */
// =============================================================================
void HF::Application::Transport::initialize ()
{
   uv_loop_t *loop = uv_default_loop ();

   uv_tcp_init (loop, &socket);

   socket.data = this;

   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   struct sockaddr_in bind_addr = uv_ip4_addr ("127.0.0.1", 8000);

   if (uv_tcp_bind (&socket, bind_addr) != 0)
   {
      print_error (loop->last_err);
      exit (-1);
   }

   if (uv_listen ((uv_stream_t *) &socket, 128, on_connect) != 0)
   {
      print_error (loop->last_err);
      exit (-2);
   }
}

#endif

#ifdef HF_NODE_APP

static void on_connect (uv_connect_t *conn, int status)
{
   CHECK_STATUS ();

   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   uv_stream_t *stream = conn->handle;

   uv_read_start (stream, (uv_alloc_cb) alloc_buffer, on_read);

   if (uv_is_writable (stream) && uv_is_readable (stream))
   {
      LOG (INFO) << "Connected !" << NL;

      HF::Application::Transport *tsp = ((HF::Application::Transport *) stream->data);
      HF::Application::Link *link     = new HF::Application::Link (tsp, stream);

      stream->data = link;

      send_hello (stream, tsp);
   }
   else
   {
      LOG (ERROR) << "Cannot read/write to stream !" << NL;
      uv_close ((uv_handle_t *) conn->handle, NULL);
   }

   free (conn);
}

// =============================================================================
// API
// =============================================================================

// =============================================================================
// Transport::initialize
// =============================================================================
/*!
 *
 */
// =============================================================================
void HF::Application::Transport::initialize ()
{
   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   uv_tcp_init (uv_default_loop (), &socket);
   uv_tcp_keepalive (&socket, 1, 60);

   socket.data = this;

   uv_connect_t *connect   = (uv_connect_t *) calloc (1, sizeof(uv_connect_t));

   struct sockaddr_in dest = uv_ip4_addr ("127.0.0.1", 8000);

   uv_tcp_connect (connect, &socket, dest, on_connect);
}

#endif

// =============================================================================
// Transport::destroy
// =============================================================================
/*!
 *
 */
// =============================================================================
void HF::Application::Transport::destroy ()
{
   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;
}

// =============================================================================
// HF::Application::Link::send
// =============================================================================
/*!
 *
 */
// =============================================================================
void HF::Application::Link::send (HF::Common::ByteArray &array)
{
   LOG (TRACE) << __PRETTY_FUNCTION__ << NL;

   msg_t msg (DATA_MSG);

   msg.data = array;

   send_message ((uv_stream_t *) stream, msg);
}