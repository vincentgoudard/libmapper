#ifndef __MAPPER_H__
#define __MAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <mapper/mapper_db.h>
#include <mapper/mapper_types.h>

#define MAPPER_NOW ((mapper_timetag_t){0L,1L})

/*! \mainpage libmapper

This is the API documentation for libmapper, a network-based signal
mapping framework.  Please see the Modules section for detailed
information, and be sure to consult the tutorial to get started with
libmapper concepts.

 */

typedef mapper_db_device mapper_device_props;
typedef mapper_db_signal mapper_signal_props;

/*** Signals ***/

/*! @defgroup signals Signals

    @{ Signals define inputs or outputs for devices.  A signal
       consists of a scalar or vector value of some integer or
       floating-point type.  A mapper_signal is created by adding an
       input or output to a device.  It can optionally be provided
       with some metadata such as a signal's range, unit, or other
       properties.  Signals can be mapped by creating maps through a
       GUI. */

struct _mapper_signal;
typedef struct _mapper_signal *mapper_signal;
struct _mapper_map;

/*! The set of possible directions for a signal or mapping slot. */
typedef enum {
    DI_OUTGOING = 0x01,
    DI_INCOMING = 0x02,
    DI_BOTH     = 0x03,
} mapper_direction_t;

/*! The set of possible actions on an instance, used to register callbacks
 *  to inform them of what is happening. */
typedef enum {
    IN_NEW                  = 0x01, //!< New instance has been created.
    IN_UPSTREAM_RELEASE     = 0x02, //!< Instance has been released by upstream device.
    IN_DOWNSTREAM_RELEASE   = 0x04, //!< Instance has been released by downstream device.
    IN_OVERFLOW             = 0x08, //!< No local instances left for incoming remote instance.
    IN_ALL                  = 0xFF
} msig_instance_event_t;

/*! A signal handler function can be called whenever a signal value
 *  changes. */
typedef void mapper_signal_update_handler(mapper_signal msig,
                                          mapper_db_signal props,
                                          int instance_id,
                                          void *value,
                                          int count,
                                          mapper_timetag_t *tt);

/*! A handler function to be called whenever a signal instance management
 *  event occurs. */
typedef void mapper_signal_instance_event_handler(mapper_signal msig,
                                                  mapper_db_signal props,
                                                  int instance_id,
                                                  msig_instance_event_t event,
                                                  mapper_timetag_t *tt);

/*! Update the value of a signal.
 *  The signal will be routed according to external requests.
 *  \param sig      The signal to update.
 *  \param value    A pointer to a new value for this signal.  If the
 *                  signal type is 'i', this should be int*; if the signal type
 *                  is 'f', this should be float*.  It should be an array at
 *                  least as long as the signal's length property.
 *  \param count    The number of instances of the value that are being
 *                  updated.  For non-periodic signals, this should be 0
 *                  or 1.  For periodic signals, this may indicate that a
 *                  block of values should be accepted, where the last
 *                  value is the current value.
 *  \param timetag  The time at which the value update was aquired. If
 *                  the value is MAPPER_NOW, libmapper will tag the
 *                  value update with the current time. See
 *                  mdev_start_queue() for more information on
 *                  bundling multiple signal updates with the same
 *                  timetag. */
void msig_update(mapper_signal sig, void *value,
                 int count, mapper_timetag_t tt);

/*! Update the value of a scalar signal of type int.
 *  This is a scalar equivalent to msig_update(), for when passing by
 *  value is more convenient than passing a pointer.
 *  The signal will be routed according to external requests.
 *  \param sig   The signal to update.
 *  \param value A new scalar value for this signal. */
void msig_update_int(mapper_signal sig, int value);

/*! Update the value of a scalar signal of type float.
 *  This is a scalar equivalent to msig_update(), for when passing by
 *  value is more convenient than passing a pointer.
 *  The signal will be routed according to external requests.
 *  \param sig The signal to update.
 *  \param value A new scalar value for this signal. */
void msig_update_float(mapper_signal sig, float value);

/*! Update the value of a scalar signal of type double.
 *  This is a scalar equivalent to msig_update(), for when passing by
 *  value is more convenient than passing a pointer.
 *  The signal will be routed according to external requests.
 *  \param sig The signal to update.
 *  \param value A new scalar value for this signal. */
void msig_update_double(mapper_signal sig, double value);

/*! Get a signal's value.
 *  \param sig      The signal to operate on.
 *  \param timetag  A location to receive the value's time tag.
 *                  May be 0.
 *  \return         A pointer to an array containing the value
 *                  of the signal, or 0 if the signal has no value. */
void *msig_value(mapper_signal sig, mapper_timetag_t *tt);

/*! Query the values of any signals connected via mapping connections.
 *  \param sig      A local output signal. We will be querying the remote
 *                  ends of this signal's mapping connections.
 *  \param tt       A timetag to be attached to the outgoing query. Query
 *                  responses should also be tagged with this time.
 *  \return The number of queries sent, or -1 for error. */
int msig_query_remotes(mapper_signal sig, mapper_timetag_t tt);

/**** Instances ****/

/*! Add new instances to the reserve list. Note that if instance ids are specified,
 *  libmapper will not add multiple instances with the same id.
 *  \param sig          The signal to which the instances will be added.
 *  \param num          The number of instances to add.
 *  \param ids          Array of integer ids, one for each new instance,
 *                      or 0 for automatically-generated instance ids.
 *  \param user_data    Array of user context pointers, one for each new instance,
 *                      or 0 if not needed.
 *  \return             Number of instances added. */
int msig_reserve_instances(mapper_signal sig, int num, int *ids, void **user_data);

/*! Update the value of a specific signal instance.
 *  The signal will be routed according to external requests.
 *  \param sig          The signal to operate on.
 *  \param instance_id  The instance to update.
 *  \param value        A pointer to a new value for this signal.  If the
 *                      signal type is 'i', this should be int*; if the signal type
 *                      is 'f', this should be float*.  It should be an array at
 *                      least as long as the signal's length property.
 *  \param count        The number of values being updated, or 0 for
 *                      non-periodic signals.
 *  \param timetag      The time at which the value update was aquired. If NULL,
 *                      libmapper will tag the value update with the current
 *                      time. See mdev_start_queue() for more information on
 *                      bundling multiple signal updates with the same timetag. */
void msig_update_instance(mapper_signal sig, int instance_id,
                          void *value, int count, mapper_timetag_t tt);

/*! Release a specific instance of a signal by removing it from the list
 *  of active instances and adding it to the reserve list.
 *  \param sig         The signal to operate on.
 *  \param instance_id The instance to suspend.
 *  \param timetag     The time at which the instance was released; if NULL,
 *                     will be tagged with the current time.
 *                     See mdev_start_queue() for more information on
 *                     bundling multiple signal updates with the same timetag. */
void msig_release_instance(mapper_signal sig, int instance_id,
                           mapper_timetag_t tt);

/*! Remove a specific instance of a signal and free its memory.
 *  \param sig         The signal to operate on.
 *  \param instance_id The instance to suspend. */
void msig_remove_instance(mapper_signal sig, int instance_id);

/*! Get the local id of the oldest active instance.
 *  \param sig         The signal to operate on.
 *  \param instance_id A location to receive the instance id.
 *  \return            Zero if an instance id has been found, non-zero otherwise. */
int msig_get_oldest_active_instance(mapper_signal sig, int *instance_id);

/*! Get the local id of the newest active instance.
 *  \param sig         The signal to operate on.
 *  \param instance_id A location to receive the instance id.
 *  \return            Zero if an instance id has been found, non-zero otherwise. */
int msig_get_newest_active_instance(mapper_signal sig, int *instance_id);

/*! Get a signal_instance's value.
 *  \param sig         The signal to operate on.
 *  \param instance_id The instance to operate on.
 *  \param timetag     A location to receive the value's time tag.
 *                     May be 0.
 *  \return            A pointer to an array containing the value
 *                     of the signal instance, or 0 if the signal instance
 *                     has no value. */
void *msig_instance_value(mapper_signal sig, int instance_id,
                          mapper_timetag_t *tt);

/*! Copy group/routing data for sharing an instance abstraction
 *  between multiple signals.
 *  \param from         The signal to copy from.
 *  \param to           The signal to copy to.
 *  \param instance_id  The instance to copy. */
void msig_match_instances(mapper_signal from, mapper_signal to, int instance_id);

/*! Return the number of active instances owned by a signal.
 *  \param  sig The signal to query.
 *  \return     The number of active instances. */
int msig_num_active_instances(mapper_signal sig);

/*! Return the number of reserved instances owned by a signal.
 *  \param  sig The signal to query.
 *  \return     The number of active instances. */
int msig_num_reserved_instances(mapper_signal sig);

/*! Get an active signal instance's ID by its index.  Intended to be
 * used for iterating over the active instances.
 *  \param sig    The signal to operate on.
 *  \param index  The numerical index of the ID to retrieve.  Shall be
 *                between zero and the return value of
 *                msig_num_active_instances().
 *  \return       The instance ID associated with the given index. */
int msig_active_instance_id(mapper_signal sig, int index);

/*! Set the allocation method to be used when a previously-unseen
 *  instance ID is received.
 *  \param sig  The signal to operate on.
 *  \param mode Method to use for adding or reallocating active instances
 *              if no reserved instances are available. */
void msig_set_instance_allocation_mode(mapper_signal sig,
                                       mapper_instance_allocation_type mode);

/*! Get the allocation method to be used when a previously-unseen
 *  instance ID is received.
 *  \param sig  The signal to operate on.
 *  \return     The allocation mode of the provided signal. */
mapper_instance_allocation_type msig_get_instance_allocation_mode(mapper_signal sig);

/*! Set the handler to be called on signal instance management events.
 *  \param sig          The signal to operate on.
 *  \param h            A handler function for processing instance managment events.
 *  \param flags        Bitflags for indicating the types of events which should
 *                      trigger the callback. Can be a combination of IN_NEW,
 *                      IN_UPSTREAM_RELEASE, IN_DOWNSTREAM_RELEASE, and IN_OVERFLOW.
 *  \param user_data    User context pointer to be passed to handler. */
void msig_set_instance_event_callback(mapper_signal sig,
                                      mapper_signal_instance_event_handler h,
                                      int flags, void *user_data);

/*! Associate a signal instance with an arbitrary pointer.
 *  \param sig          The signal to operate on.
 *  \param instance_id  The instance to operate on.
 *  \param user_data    A pointer to user data to be associated
 *                      with this instance. */
void msig_set_instance_data(mapper_signal sig, int instance_id,
                            void *user_data);

/*! Retrieve the arbitrary pointer associated with a signal instance.
 *  \param sig          The signal to operate on.
 *  \param instance_id  The instance to operate on.
 *  \return             A pointer associated with this instance. */
void *msig_get_instance_data(mapper_signal sig, int instance_id);

/*! Set or unset the message handler for a signal.
 *  \param sig       The signal to operate on.
 *  \param handler   A pointer to a mapper_signal_update_handler
 *                   function for processing incoming messages.
 *  \param user_data User context pointer to be passed to handler. */
void msig_set_callback(mapper_signal sig,
                       mapper_signal_update_handler *handler,
                       void *user_data);

/*! Get the number of maps attatched to a specific signal.
 *  \param sig      The signal to check.
 *  \return         The number of attached maps. */
int msig_num_maps(mapper_signal sig);

/**** Signal properties ****/

/*! Get the full OSC name of a signal, including device name
 *  prefix.
 *  \param sig  The signal value to query.
 *  \param name A string to accept the name.
 *  \param len  The length of string pointed to by name.
 *  \return The number of characters used, or 0 if error.  Note that
 *          in some cases the name may not be available. */
int msig_full_name(mapper_signal sig, char *name, int len);

/*! Set or remove the minimum of a signal.
 *  \param sig      The signal to operate on.
 *  \param minimum  Must be the same type as the signal, or 0 to remove
 *                  the minimum. */
void msig_set_minimum(mapper_signal sig, void *minimum);

/*! Set or remove the maximum of a signal.
 *  \param sig      The signal to operate on.
 *  \param maximum  Must be the same type as the signal, or 0 to remove
 *                  the maximum. */
void msig_set_maximum(mapper_signal sig, void *maximum);

/*! Set the rate of a signal.
 *  \param sig      The signal to operate on.
 *  \param rate     A rate for this signal in samples/second, or zero
 *                  for non-periodic signals. */
void msig_set_rate(mapper_signal sig, float rate);

/*! Set the direction of a signal.
 *  \param sig      The signal to operate on.
 *  \param rate     DI_OUTGOING, DI_INCOMING, or DI_BOTH. */
void msig_set_direction(mapper_signal sig, mapper_direction_t direction);

/*! Get a signal's property structure.
 *  \param sig  The signal to operate on.
 *  \return     A structure containing the signal's properties. */
mapper_db_signal msig_properties(mapper_signal sig);

/*! Set a property of a signal.  Can be used to provide arbitrary
 *  metadata.  Value pointed to will be copied.
 *  \param sig       The signal to operate on.
 *  \param property  The name of the property to add.
 *  \param type      The property  datatype.
 *  \param value     An array of property values.
 *  \param length    The length of value array. */
void msig_set_property(mapper_signal sig, const char *property,
                       char type, void *value, int length);

/*! Look up a device property by name.
 *  \param sig      The signal to look at.
 *  \param property The name of the property to retrieve.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int msig_property_lookup(mapper_signal sig, const char *property,
                         char *type, const void **value, int *length);

/*! Remove a property of a signal.
 *  \param sig       The signal to operate on.
 *  \param property  The name of the property to remove. */
void msig_remove_property(mapper_signal sig, const char *property);

/* @} */

/*** Devices ***/

/*! @defgroup devices Devices

    @{ A device is an entity on the network which has input and/or output
       signals.  The mapper_device is the primary interface through which a
       program uses libmapper.  A device must have a name, to which a unique
       ordinal is subsequently appended.  It can also be given other
       user-specified metadata.  Devices signals can be connected, which is
       accomplished by requests from an external GUI. */

/*! Allocate and initialize a mapper device.
 *  \param name_prefix  A short descriptive string to identify the device.
 *  \param port         An optional port for starting the port allocation
 *                      scheme.
 *  \param admin        A previously allocated admin to use.  If 0, an
 *                      admin will be allocated for use with this device.
 *  \return             A newly allocated mapper device.  Should be free
 *                      using mdev_free(). */
mapper_device mdev_new(const char *name_prefix, int port, mapper_admin admin);

//! Free resources used by a mapper device.
void mdev_free(mapper_device dev);

/*! Add an input signal to a mapper device.  Values and strings
 *  pointed to by this call (except user_data) will be copied.
 *  For minimum and maximum, actual type must correspond to 'type'.
 *  If type='i', then int*; if type='f', then float*.
 *  \param dev     The device to add a signal to.
 *  \param name    The name of the signal.
 *  \param length  The length of the signal vector, or 1 for a scalar.
 *  \param type    The type fo the signal value.
 *  \param unit    The unit of the signal, or 0 for none.
 *  \param minimum Pointer to a minimum value, or 0 for none.
 *  \param maximum Pointer to a maximum value, or 0 for none.
 *  \param handler Function to be called when the value of the
 *                 signal is updated.
 *  \param user_data User context pointer to be passed to handler. */
mapper_signal mdev_add_input(mapper_device dev, const char *name,
                             int length, char type, const char *unit,
                             void *minimum, void *maximum,
                             mapper_signal_update_handler *handler,
                             void *user_data);

/*! Add an output signal to a mapper device.  Values and strings
 *  pointed to by this call will be copied.
 *  For minimum and maximum, actual type must correspond to 'type'.
 *  If type='i', then int*; if type='f', then float*.
 *  \param dev     The device to add a signal to.
 *  \param name    The name of the signal.
 *  \param length  The length of the signal vector, or 1 for a scalar.
 *  \param type    The type fo the signal value.
 *  \param unit    The unit of the signal, or 0 for none.
 *  \param minimum Pointer to a minimum value, or 0 for none.
 *  \param maximum Pointer to a maximum value, or 0 for none. */
mapper_signal mdev_add_output(mapper_device dev, const char *name,
                              int length, char type, const char *unit,
                              void *minimum, void *maximum);

/* Remove a device's signal.
 * \param dev The device to remove a signal from.
 * \param sig The signal to remove. */
void mdev_remove_signal(mapper_device dev, mapper_signal sig);

/* Remove a device's input signal.
 * \param dev The device to remove a signal from.
 * \param sig The signal to remove. */
void mdev_remove_input(mapper_device dev, mapper_signal sig);

/* Remove a device's output signal.
 * \param dev The device to remove a signal from.
 * \param sig The signal to remove. */
void mdev_remove_output(mapper_device dev, mapper_signal sig);

//! Return the number of inputs.
int mdev_num_inputs(mapper_device dev);

//! Return the number of outputs.
int mdev_num_outputs(mapper_device dev);

//! Return the number of incoming maps.
int mdev_num_incoming_maps(mapper_device dev);

//! Return the number of outgoing maps.
int mdev_num_outgoing_maps(mapper_device dev);

/*! Get input signals.
 *  \param dev Device to search in.
 *  \return Pointer to the linked list of input mapper_signals, or zero
 *          if not found.
 */
mapper_signal *mdev_get_inputs(mapper_device dev);

/*! Get output signals.
 *  \param dev Device to search in.
 *  \return Pointer to the linked list of output mapper_signals, or zero
 *          if not found.
 */
mapper_signal *mdev_get_outputs(mapper_device dev);

/*! Get a signal with a given name.
 *  \param dev   Device to search in.
 *  \param name  Name of the signal to search for. It may optionally
 *               begin with a '/' character.
 *  \param index Optional place to receive the index of the matching signal.
 *  \return Pointer to the mapper_signal describing the signal, or zero
 *          if not found.
 */
mapper_signal mdev_get_signal_by_name(mapper_device dev, const char *name,
                                      int *index);

/*! Get an input signal with a given name.
 *  \param dev   Device to search in.
 *  \param name  Name of input signal to search for. It may optionally
 *               begin with a '/' character.
 *  \param index Optional place to receive the index of the matching signal.
 *  \return Pointer to the mapper_signal describing the signal, or zero
 *          if not found.
 */
mapper_signal mdev_get_input_by_name(mapper_device dev, const char *name,
                                     int *index);

/*! Get an output signal with a given name.
 *  \param dev   Device to search in.
 *  \param name  Name of output signal to search for. It may optionally
 *               begin with a '/' character.
 *  \param index Optional place to receive the index of the matching signal.
 *  \return Pointer to the mapper_signal describing the signal, or zero
 *          if not found.
 */
mapper_signal mdev_get_output_by_name(mapper_device dev, const char *name,
                                      int *index);

/* Get an input signal with the given index.
 * \param dev   The device to search in.
 * \param index Index of the signal to retrieve.
 * \return Pointer to the mapper_signal describing the signal, or zero
 *         if not found. */
mapper_signal mdev_get_input_by_index(mapper_device dev, int index);

/* Get an output signal with the given index.
 * \param dev   The device to search in.
 * \param index Index of the signal to retrieve.
 * \return Pointer to the mapper_signal describing the signal, or zero
 *         if not found. */
mapper_signal mdev_get_output_by_index(mapper_device dev, int index);

/*! Get a device's property structure.
 *  \param dev  The device to operate on.
 *  \return     A structure containing the device's properties. */
mapper_device_props mdev_properties(mapper_device dev);

/*! Set a property of a device.  Can be used to provide arbitrary
 *  metadata.  Value pointed to will be copied.
 *  \param dev       The device to operate on.
 *  \param property  The name of the property to add.
 *  \param type      The property  datatype.
 *  \param value     An array of property values.
 *  \param length    The length of value array. */
void mdev_set_property(mapper_device dev, const char *property,
                       char type, void *value, int length);

/*! Look up a device property by name.
 *  \param dev      The device to look at.
 *  \param property The name of the property to retrieve.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mdev_property_lookup(mapper_device dev, const char *property,
                         char *type, const void **value, int *length);

/*! Remove a property of a device.
 *  \param dev       The device to operate on.
 *  \param property  The name of the property to remove. */
void mdev_remove_property(mapper_device dev, const char *property);

/*! Poll this device for new messages.
 *  Note, if you have multiple devices, the right thing to do is call
 *  this function for each of them with block_ms=0, and add your own
 *  sleep if necessary.
 *  \param dev      The device to check messages for.
 *  \param block_ms Number of milliseconds to block waiting for
 *                  messages, or 0 for non-blocking behaviour.
 *  \return The number of handled messages. May be zero if there was
 *          nothing to do. */
int mdev_poll(mapper_device dev, int block_ms);

/*! Return the number of file descriptors needed for this device.
 *  This can be used to allocated an appropriately-sized list for
 *  called to mdev_get_fds.  Note that the number of descriptors
 *  needed can change throughout the life of a device, therefore this
 *  function should be called whenever the list of file descriptors is
 *  needed.
 *  \param md The device to count file descriptors for.
 *  \return The number of file descriptors needed for the indicated
 *          device. */
int mdev_num_fds(mapper_device md);

/*! Write the list of file descriptors for this device to the provided
 *  array.  Up to num file descriptors will be written.  These file
 *  descriptors can be used as input for the read array of select or
 *  poll, for example.
 *  \param md  The device to get file descriptors for.
 *  \param fds Memory to receive file descriptors.
 *  \param num The number of file descriptors pointed to by fds.
 *  \return The number of file descriptors actually written to fds. */
int mdev_get_fds(mapper_device md, int *fds, int num);

/*! If an external event indicates that a file descriptor for this
 *  device needs servicing, this function should be called.
 *  \param md The device that needs servicing.
 *  \param fd The file descriptor that needs servicing. */
void mdev_service_fd(mapper_device md, int fd);

/*! Detect whether a device is completely initialized.
 *  \return Non-zero if device is completely initialized, i.e., has an
 *  allocated receiving port and unique network name.  Zero
 *  otherwise. */
int mdev_ready(mapper_device dev);

/*! Return a string indicating the device's full name, if it is
 *  registered.  The returned string must not be externally modified.
 *  \param dev The device to query.
 *  \return String containing the device's full name, or zero if it is
 *  not available. */
const char *mdev_name(mapper_device dev);

/*! Return the unique hash allocated to this device by the mapper network.
 *  \param  dev The device to query.
 *  \return An integer indicating the device's hash, or zero if it is
 *          not available. */
unsigned int mdev_hash(mapper_device dev);

/*! Return the port used by a device to receive signals, if available.
 *  \param dev The device to query.
 *  \return An integer indicating the device's port, or zero if it is
 *  not available. */
unsigned int mdev_port(mapper_device dev);

/*! Return the IPv4 address used by a device to receive signals, if available.
 *  \param dev The device to query.
 *  \return A pointer to an in_addr struct indicating the device's IP
 *          address, or zero if it is not available.  In general this
 *          will be the IPv4 address associated with the selected
 *          local network interface.
 */
const struct in_addr *mdev_ip4(mapper_device dev);

/*! Return a string indicating the name of the network interface this
 *  device is listening on.
 *  \param dev The device to query.
 *  \return A string containing the name of the network interface. */
const char *mdev_interface(mapper_device dev);

/*! Return an allocated ordinal which is appended to this device's
 *  network name.  In general the results of this function are not
 *  valid unless mdev_ready() returns non-zero.
 *  \param dev The device to query.
 *  \return A positive ordinal unique to this device (per name). */
unsigned int mdev_ordinal(mapper_device dev);

/*! Start a time-tagged mapper queue. */
void mdev_start_queue(mapper_device md, mapper_timetag_t tt);

/*! Dispatch a time-tagged mapper queue. */
void mdev_send_queue(mapper_device md, mapper_timetag_t tt);

/*! Get access to the device's underlying lo_server. */
lo_server mdev_get_lo_server(mapper_device md);

/*! Get the device's synchronization clock offset. */
double mdev_get_clock_offset(mapper_device md);

/*! The set of possible actions on a local device map. */
typedef enum {
    MDEV_LOCAL_ESTABLISHED,
    MDEV_LOCAL_MODIFIED,
    MDEV_LOCAL_DESTROYED,
} mapper_device_local_action_t;

/*! Function to call when a local device map is established or
 *  destroyed. */
typedef void mapper_device_map_handler(mapper_device dev, mapper_signal sig,
                                       mapper_db_map map, mapper_db_map_slot slot,
                                       mapper_device_local_action_t action,
                                       void *user);

/*! Set a function to be called when a map involving the local device is
 *  established or destroyed, indicated by the action parameter to the
 *  provided function. */
void mdev_set_map_callback(mapper_device dev, mapper_device_map_handler *h,
                           void *user);

/* @} */

/*** Admins ***/

/*! @defgroup admins Admins

    @{ Admins handle the traffic on the multicast admin bus.  In
       general, you do not need to worry about this interface, as an
       admin will be created automatically when allocating a device.
       An admin only needs to be explicitly created if you plan to
       override default settings for the admin bus.  */

/*! Create an admin with custom parameters.  Creating an admin object
 *  manually is only required if you wish to specify custom network
 *  parameters.  Creating a device or monitor without specifying an
 *  admin will give you an object working on the "standard"
 *  configuration.
 * \param iface  If non-zero, a string identifying a preferred network
 *               interface.  This string can be enumerated e.g. using
 *               if_nameindex(). If zero, an interface will be
 *               selected automatically.
 * \param group  If non-zero, specify a multicast group address to use.
 *               Zero indicates that the standard group 224.0.1.3 should
 *               be used.
 * \param port   If non-zero, specify a multicast port to use.  Zero
 *               indicates that the standard port 7570 should be used.
 * \return       A newly allocated admin.  Should be freed using
 *               mapper_admin_free() */
mapper_admin mapper_admin_new(const char *iface, const char *group, int port);

/*! Free an admin created with mapper_admin_new(). */
void mapper_admin_free(mapper_admin admin);

/*! Get the version of libmapper */
const char *mapper_admin_libversion(mapper_admin admin);

/* @} */

/**** Device database ****/

/*! @defgroup devicedb Device database

    @{ A monitor may query information about devices on the network
       through this interface. */

/*! The set of possible actions on a database record, used
 *  to inform callbacks of what is happening to a record. */
typedef enum {
    MDB_MODIFY,
    MDB_NEW,
    MDB_REMOVE,
    MDB_UNRESPONSIVE,
} mapper_db_action_t;

/*! A callback function prototype for when a device record is added or
 *  updated in the database. Such a function is passed in to
 *  mapper_db_add_device_callback().
 *  \param record  Pointer to the device record.
 *  \param action  A value of mapper_db_action_t indicating what
 *                 is happening to the database record.
 *  \param user    The user context pointer registered with this
 *                 callback. */
typedef void mapper_db_device_handler(mapper_db_device record,
                                      mapper_db_action_t action,
                                      void *user);

/*! Register a callback for when a device record is added or updated
 *  in the database.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user A user-defined pointer to be passed to the callback
 *              for context . */
void mapper_db_add_device_callback(mapper_db db,
                                   mapper_db_device_handler *h,
                                   void *user);

/*! Remove a device record callback from the database service.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user The user context pointer that was originally specified
 *              when adding the callback. */
void mapper_db_remove_device_callback(mapper_db db,
                                      mapper_db_device_handler *h,
                                      void *user);

/*! Return the whole list of devices.
 *  \param db   The database to query.
 *  \return A double-pointer to the first item in the list of devices,
 *          or zero if none.  Use mapper_db_device_next() to
 *          iterate. */
mapper_db_device_t **mapper_db_get_all_devices(mapper_db db);

/*! Find information for a registered device.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find in the database.
 *  \return            Information about the device, or zero if not found. */
mapper_db_device mapper_db_get_device_by_name(mapper_db db,
                                              const char *device_name);

/*! Look up information for a registered device using a hash of its name.
 *  \param db   The database to query.
 *  \param hash CRC-32 name hash of the device to find in the database.
 *  \return     Information about the device, or zero if not found. */
mapper_db_device mapper_db_get_device_by_hash(mapper_db db, uint32_t hash);

/*! Return the list of devices with a substring in their name.
 *  \param db             The database to query.
 *  \param device_pattern The substring to search for.
 *  \return    A double-pointer to the first item in a list of matching
 *             devices.  Use mapper_db_device_next() to iterate. */
mapper_db_device_t **mapper_db_match_devices_by_name(mapper_db db,
                                                     const char *device_pattern);

/*! Given a device record pointer returned from a previous
 *  mapper_db_return_*() call, get the next item in the list.
 *  \param s The previous device record pointer.
 *  \return  A double-pointer to the next device record in the list, or
 *           zero if no more devices. */
mapper_db_device_t **mapper_db_device_next(mapper_db_device_t **s);

/*! Given a device record pointer returned from a previous
 *  mapper_db_get_*() call, indicate that we are done iterating.
 *  \param s The previous device record pointer. */
void mapper_db_device_done(mapper_db_device_t **s);


/*! Look up a device property by index. To iterate all properties,
 *  call this function from index=0, increasing until it returns zero.
 *  \param dev      The device to look at.
 *  \param index    Numerical index of a device property.
 *  \param property Address of a string pointer to receive the name of
 *                  indexed property.  May be zero.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mapper_db_device_property_index(mapper_db_device dev, unsigned int index,
                                    const char **property, char *type,
                                    const void **value, int *length);

/*! Look up a device property by name.
 *  \param dev      The device to look at.
 *  \param property The name of the property to retrieve.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mapper_db_device_property_lookup(mapper_db_device dev,
                                     const char *property, char *type,
                                     const void **value, int *length);

/*! Helper for printing typed mapper_prop values.
 *  \param type     The value type.
 *  \param length   The vector length of the value.
 *  \param value    A pointer to the property value to print. */
void mapper_prop_pp(char type, int length, const void *value);

/* @} */

/***** Signals *****/

/*! @defgroup signaldb Signal database

    @{ A monitor may query information about signals on the network
       through this interface. It is also used by local signals to
       store property information. */

/*! A callback function prototype for when a signal record is added or
 *  updated in the database. Such a function is passed in to
 *  mapper_db_add_signal_callback().
 *  \param record  Pointer to the signal record.
 *  \param action  A value of mapper_db_action_t indicating what
 *                 is happening to the database record.
 *  \param user    The user context pointer registered with this
 *                 callback. */
typedef void mapper_db_signal_handler(mapper_db_signal record,
                                      mapper_db_action_t action,
                                      void *user);

/*! Register a callback for when a signal record is added or updated
 *  in the database.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user A user-defined pointer to be passed to the callback
 *              for context . */
void mapper_db_add_signal_callback(mapper_db db,
                                   mapper_db_signal_handler *h,
                                   void *user);

/*! Remove a signal record callback from the database service.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user The user context pointer that was originally specified
 *              when adding the callback. */
void mapper_db_remove_signal_callback(mapper_db db,
                                      mapper_db_signal_handler *h,
                                      void *user);

/*! Return the list of all known inputs across all devices.
 *  \param db   The database to query.
 *  \return A double-pointer to the first item in the list of results
 *          or zero if none.  Use mapper_db_signal_next() to iterate. */
mapper_db_signal_t **mapper_db_get_all_inputs(mapper_db db);

/*! Return the list of all known outputs across all devices.
 *  \param db   The database to query.
 *  \return A double-pointer to the first item in the list of results
 *          or zero if none.  Use mapper_db_signal_next() to iterate. */
mapper_db_signal_t **mapper_db_get_all_outputs(mapper_db db);

/*! Return the list of signals for a given device.
 *  \param db          The database to query.
 *  \param device_name Name of the device to match for signals.  Must
 *                     be exact, including the leading '/'.
 *  \return A double-pointer to the first item in the list of
 *          signals, or zero if none.  Use mapper_db_signal_next() to
 *          iterate. */
mapper_db_signal_t **mapper_db_get_signals_by_device_name(
    mapper_db db, const char *device_name);

/*! Return the list of inputs for a given device.
 *  \param db          The database to query.
 *  \param device_name Name of the device to match for outputs.  Must
 *                     be exact, including the leading '/'.
 *  \return A double-pointer to the first item in the list of input
 *          signals, or zero if none.  Use mapper_db_signal_next() to
 *          iterate. */
mapper_db_signal_t **mapper_db_get_inputs_by_device_name(
    mapper_db db, const char *device_name);

/*! Return the list of outputs for a given device.
 *  \param db          The database to query.
 *  \param device_name Name of the device to match for outputs.  Must
 *                     be exact, including the leading '/'.
 *  \return A double-pointer to the first item in the list of output
 *          signals, or zero if none.  Use mapper_db_signal_next() to
 *          iterate. */
mapper_db_signal_t **mapper_db_get_outputs_by_device_name(
    mapper_db db, const char *device_name);

/*! Find information for a registered input signal.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find in the database.
 *  \param signal_name Name of the signal to find in the database.
 *  \return            Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_signal_by_device_and_signal_names(
    mapper_db db, const char *device_name, const char *signal_name);

/*! Find information for a registered input signal.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find in the database.
 *  \param signal_name Name of the input signal to find in the database.
 *  \return            Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_input_by_device_and_signal_names(
    mapper_db db, const char *device_name, const char *signal_name);

/*! Find information for a registered output signal.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find in the database.
 *  \param signal_name Name of the output signal to find in the database.
 *  \return            Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_output_by_device_and_signal_names(
    mapper_db db, const char *device_name, char const *signal_name);

/*! Find information for a registered input signal.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to find in the database.
 *  \param index        Index of the signal to find in the database.
 *  \return             Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_signal_by_device_name_and_index(
    mapper_db db, const char *device_name, int index);

/*! Find information for a registered input signal.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to find in the database.
 *  \param index        Index of the input signal to find in the database.
 *  \return             Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_input_by_device_name_and_index(
    mapper_db db, const char *device_name, int index);

/*! Find information for a registered output signal.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to find in the database.
 *  \param index        Index of the output signal to find in the database.
 *  \return             Information about the signal, or zero if not found. */
mapper_db_signal mapper_db_get_output_by_device_name_and_index(
    mapper_db db, const char *device_name, int index);

/*! Return the list of inputs for a given device.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to match for inputs.
 *  \param pattern      A substring to search for in the device inputs.
 *  \return A double-pointer to the first item in the list of input signals,
 *          or zero if none.  Use mapper_db_signal_next() to iterate. */
mapper_db_signal_t **mapper_db_match_inputs_by_device_name(
    mapper_db db, const char *device_name, const char *pattern);

/*! Return the list of outputs for a given device.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to match for outputs.
 *  \param pattern      A substring to search for in the device outputs.
 *  \return A double-pointer to the first item in the list of output signals,
 *          or zero if none.  Use mapper_db_signal_next() to iterate. */
mapper_db_signal_t **mapper_db_match_outputs_by_device_name(
    mapper_db db, const char *device_name, char const *pattern);

/*! Return the list of signals for a given device.
 *  \param db           The database to query.
 *  \param device_name  Name of the device to match for signals.
 *  \param pattern      A substring to search for in the device signals.
 *  \return A double-pointer to the first item in the list of signals, or zero
 *          if none.  Use mapper_db_signal_next() to iterate. */
mapper_db_signal_t **mapper_db_match_signals_by_device_name(
    mapper_db db, const char *device_name, char const *pattern);

/*! Given a signal record pointer returned from a previous
 *  mapper_db_get_*() call, get the next item in the list.
 *  \param  s The previous signal record pointer.
 *  \return A double-pointer to the next signal record in the list, or
 *          zero if no more signals. */
mapper_db_signal_t **mapper_db_signal_next(mapper_db_signal_t **s);

/*! Given a signal record pointer returned from a previous
 *  mapper_db_get_*() call, indicate that we are done iterating.
 *  \param s The previous signal record pointer. */
void mapper_db_signal_done(mapper_db_signal_t **s);

/*! Look up a signal property by index. To iterate all properties,
 *  call this function from index=0, increasing until it returns zero.
 *  \param sig      The signal to look at.
 *  \param index    Numerical index of a signal property.
 *  \param property Address of a string pointer to receive the name of
 *                  indexed property.  May be zero.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mapper_db_signal_property_index(mapper_db_signal sig, unsigned int index,
                                    const char **property, char *type,
                                    const void **value, int *length);

/*! Look up a signal property by name.
 *  \param sig      The signal to look at.
 *  \param property The name of the property to retrive.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mapper_db_signal_property_lookup(mapper_db_signal sig,
                                     const char *property, char *type,
                                     const void **value, int *length);

/* @} */

/***** Maps *****/

/*! @defgroup mapdb Maps database

    @{ A monitor may query information about maps between
       signals on the network through this interface.  It is also used
       to specify properties during mapping requests. */

/*! A callback function prototype for when a map record is
 *  added or updated in the database. Such a function is passed in to
 *  mapper_db_add_map_callback().
 *  \param record  Pointer to the map record.
 *  \param action  A value of mapper_db_action_t indicating what
 *                 is happening to the database record.
 *  \param user    The user context pointer registered with this
 *                 callback. */
typedef void mapper_db_map_handler(mapper_db_map record,
                                   mapper_db_action_t action, void *user);

/*! Register a callback for when a map record is added or
 *  updated in the database.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user A user-defined pointer to be passed to the callback
 *              for context . */
void mapper_db_add_map_callback(mapper_db db, mapper_db_map_handler *h,
                                void *user);

/*! Remove a map record callback from the database service.
 *  \param db   The database to query.
 *  \param h    Callback function.
 *  \param user The user context pointer that was originally specified
 *              when adding the callback. */
void mapper_db_remove_map_callback(mapper_db db, mapper_db_map_handler *h,
                                   void *user);

/*! Return a list of all registered maps.
 *  \param db The database to query.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_all_maps(mapper_db db);

/*! Return the map that match the given map hash.
 *  \param db          The database to query.
 *  \param hash        map hash.
 *  \return A pointer to a structure containing information on the
 *          found map, or 0 if not found. */
mapper_db_map mapper_db_get_map_by_hash(mapper_db db, uint32_t hash);

/*! Return the map that match the given id.
 *  \param db          The database to query.
 *  \param device_name Name of destination device.
 *  \param int         ID of the map to retrieve.
 *  \return A pointer to a structure containing information on the
 *          found map, or 0 if not found. */
mapper_db_map mapper_db_get_map_by_dest_device_and_id(
    mapper_db db, const char *device_name, int id);

/*! Return the list of maps that touch the given device name.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_device_name(mapper_db db,
                                                    const char *device_name);

/*! Return the list of outgoing maps that touch the given device name.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_src_device_name(mapper_db db,
                                                        const char *device_name);

/*! Return the list of incoming maps that touch the given device name.
 *  \param db          The database to query.
 *  \param device_name Name of the device to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_dest_device_name(mapper_db db,
                                                         const char *device_name);

/*! Return the list of maps for a given source signal name.
 *  \param  db         The database to query.
 *  \param  src_signal Name of the source signal.
 *  \return A double-pointer to the first item in the list of results
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_src_signal_name(
    mapper_db db, const char *src_signal);

/*! Return the list of maps for given source device and signal names.
 *  \param db         The database to query.
 *  \param src_device Exact name of the device to find, including the
 *                    leading '/'.
 *  \param src_signal Exact name of the signal to find,
 *                    including the leading '/'.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_src_device_and_signal_names(
    mapper_db db, const char *src_device, const char *src_signal);

/*! Return the list of maps for a given destination signal name.
 *  \param db          The database to query.
 *  \param dest_signal Name of the destination signal to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_dest_signal_name(
    mapper_db db, const char *dest_signal);

/*! Return the list of maps for given destination device and signal names.
 *  \param db          The database to query.
 *  \param dest_device Exact name of the device to find, including the
 *                     leading '/'.
 *  \param dest_signal Exact name of the signal to find, including the
 *                     leading '/'.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_dest_device_and_signal_names(
    mapper_db db, const char *dest_device, const char *dest_signal);

/*! Return the list of maps that touch the provided signal.
 *  \param db           The database to query.
 *  \param device_name  Exact name of the device to find.
 *  \param signal_name  Exact name of the signal to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_device_and_signal_name(
    mapper_db db, const char *device_name,  const char *signal_name);

/*! Return the list of maps that touch the provided source
 *  and destination signals.
 *  \param db          The database to query.
 *  \param src_device  Exact name of the device to find.
 *  \param src_signal  Exact name of the signal to find.
 *  \param dest_device Exact name of the device to find.
 *  \param dest_signal Exact name of the signal to find.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_device_and_signal_names(
    mapper_db db, int num_sources,
    const char **src_devices,  const char **src_signals,
    const char *dest_device, const char *dest_signal);

/*! Return the map that match the exact source and destination
 *  specified by their full names ("/<device>/<signal>").
 *  \param db        The database to query.
 *  \param src_name  Full name of source signal.
 *  \param dest_name Full name of destination signal.
 *  \return A pointer to a structure containing information on the
 *          found map, or 0 if not found. */
mapper_db_map mapper_db_get_map_by_signal_full_names(
    mapper_db db, int num_sources, const char **src_names, const char *dest_name);

/*! Return maps that have the specified source and destination
 *  devices.
 *  \param db               The database to query.
 *  \param src_device_name  Name of source device.
 *  \param dest_device_name Name of destination device.
 *  \return A double-pointer to the first item in a list of results,
 *          or 0 if not found. */
mapper_db_map_t **mapper_db_get_maps_by_src_dest_device_names(
    mapper_db db, int num_sources,
    const char **src_device_names, const char *dest_device_name);

/*! Return the list of maps that touch any signals in the lists
 *  of sources and destinations provided.
 *  \param db   The database to query.
 *  \param src  Double-pointer to the first item in a list
 *              returned from a previous database query.
 *  \param dest Double-pointer to the first item in a list
 *              returned from a previous database query.
 *  \return A double-pointer to the first item in the list of results,
 *          or zero if none.  Use mapper_db_map_next() to
 *          iterate. */
mapper_db_map_t **mapper_db_get_maps_by_signal_queries(mapper_db db,
                                                       mapper_db_signal_t **src,
                                                       mapper_db_signal_t **dest);

/*! Given a map record pointer returned from a previous
 *  mapper_db_get_maps*() call, get the next item in the list.
 *  \param s The previous map record pointer.
 *  \return  A double-pointer to the next map record in the
 *           list, or zero if no more maps. */
mapper_db_map_t **mapper_db_map_next(mapper_db_map_t **s);

/*! Given a map record pointer returned from a previous
 *  mapper_db_get_*() call, indicate that we are done iterating.
 *  \param s The previous map record pointer. */
void mapper_db_map_done(mapper_db_map_t **s);

/*! Look up a map property by index. To iterate all properties,
 *  call this function from index=0, increasing until it returns zero.
 *  \param map      The map to look at.
 *  \param index    Numerical index of a map property.
 *  \param property Address of a string pointer to receive the name of
 *                  indexed property.  May be zero.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return Zero if found, otherwise non-zero. */
int mapper_db_map_property_index(mapper_db_map map, unsigned int index,
                                 const char **property, char *type,
                                 const void **value, int *length);

/*! Look up a map property by name.
 *  \param map      The map to look at.
 *  \param property The name of the property to retrive.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return         Zero if found, otherwise non-zero. */
int mapper_db_map_property_lookup(mapper_db_map map, const char *property,
                                  char *type, const void **value, int *length);

/*! Look up a map slot property by index. To iterate all properties,
 *  call this function from index=0, increasing until it returns zero.
 *  \param slot     The map slot to look at.
 *  \param index    Numerical index of a map slot property.
 *  \param property Address of a string pointer to receive the name of
 *                  indexed property.  May be zero.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return         Zero if found, otherwise non-zero. */
int mapper_db_map_slot_property_index(mapper_db_map_slot slot, unsigned int index,
                                      const char **property, char *type,
                                      const void **value, int *length);

/*! Look up a map property by name.
 *  \param slot     The map slot to look at.
 *  \param property The name of the property to retrieve.
 *  \param type     A pointer to a location to receive the type of the
 *                  property value. (Required.)
 *  \param value    A pointer to a location to receive the address of the
 *                  property's value. (Required.)
 *  \param length   A pointer to a location to receive the vector length of
 *                  the property value. (Required.)
 *  \return         Zero if found, otherwise non-zero. */
int mapper_db_map_slot_property_lookup(mapper_db_map_slot slot,
                                       const char *property, char *type,
                                       const void **value, int *length);

/***** Monitors *****/

/*! @defgroup monitor Monitors

    @{ Monitors are the primary interface through which a program may
       observe the network and store information about devices and
       signals that are present.  Each monitor has a database of
       devices, signals, and maps, which can be queried.
       A monitor can also make map requests.  In
       general, the monitor interface is useful for building GUI
       applications to control the network. */

/*! Bit flags for coordinating monitor metadata subscriptions. Subsets of
 *  device information must also include SUB_DEVICE. */
#define SUBSCRIBE_NONE              0x00
#define SUBSCRIBE_DEVICE            0x01
#define SUBSCRIBE_DEVICE_INPUTS     0x02
#define SUBSCRIBE_DEVICE_OUTPUTS    0x04
#define SUBSCRIBE_DEVICE_SIGNALS    SUBSCRIBE_DEVICE_INPUTS | SUBSCRIBE_DEVICE_OUTPUTS
#define SUBSCRIBE_DEVICE_MAPS_IN    0x10
#define SUBSCRIBE_DEVICE_MAPS_OUT   0x20
#define SUBSCRIBE_DEVICE_MAPS       SUBSCRIBE_DEVICE_MAPS_IN | SUBSCRIBE_DEVICE_MAPS_OUT
#define SUBSCRIBE_ALL               0xFF

/*! Create a network monitor.
 *  \param admin               A previously allocated admin to use.  If 0, an
 *                             admin will be allocated for use with this monitor.
 *  \param autosubscribe_flags Sets whether the monitor should automatically
 *                             subscribe to information about signals
 *                             and maps when it encounters a
 *                             previously-unseen device.
 *  \return The new monitor. */
mapper_monitor mmon_new(mapper_admin admin, int autosubscribe_flags);

/*! Free a network monitor. */
void mmon_free(mapper_monitor mon);

/*! Poll a network monitor.
 *  \param mon      The monitor to poll.
 *  \param block_ms The number of milliseconds to block, or 0 for
 *                  non-blocking behaviour.
 *  \return The number of handled messages. */
int mmon_poll(mapper_monitor mon, int block_ms);

/*! Get the database associated with a monitor. This can be used as
 *  long as the monitor remains alive. */
mapper_db mmon_get_db(mapper_monitor mon);

/*! Set the timeout in seconds after which a monitor will declare a device
 *  "unresponsive". Defaults to ADMIN_TIMEOUT_SEC.
 *  \param mon      The monitor to use.
 *  \param timeout  The timeout in seconds. */
void mmon_set_timeout(mapper_monitor mon, int timeout);

/*! Get the timeout in seconds after which a monitor will declare a device
 *  "unresponsive". Defaults to ADMIN_TIMEOUT_SEC.
 *  \param mon      The monitor to use.
 *  \return The current timeout in seconds. */
int mmon_get_timeout(mapper_monitor mon);

/*! Remove unresponsive devices from the database.
 *  \param mon         The monitor to use.
 *  \param timeout_sec The number of seconds a device must have been
 *                     unresponsive before removal.
 *  \param quiet       1 to disable callbacks during db flush, 0 otherwise. */
void mmon_flush_db(mapper_monitor mon, int timeout_sec, int quiet);

/*! Request that all devices report in. */
void mmon_request_devices(mapper_monitor mon);

/*! Subscribe to information about a specific device.
 *  \param mon             The monitor to use.
 *  \param device_name     The name of the device of interest.
 *  \param subscribe_flags Bitflags setting the type of information of interest.
 *                         Can be a combination of SUB_DEVICE, SUB_DEVICE_INPUTS,
 *                         SUB_DEVICE_OUTPUTS, SUB_DEVICE_SIGNALS,
 *                         SUB_DEVICE_MAPS_IN, SUB_DEVICE_MAPS_OUT,
 *                         SUB_DEVICE_MAPS, or simply SUB_DEVICE_ALL for all
 *                         information.
 *  \param timeout         The length in seconds for this subscription. If set
 *                         to -1, libmapper will automatically renew the
 *                         subscription until the monitor is freed or this
 *                         function is called again. */
void mmon_subscribe(mapper_monitor mon, const char *device_name,
                    int subscribe_flags, int timeout);

/*! Unsubscribe from information about a specific device.
 *  \param mon             The monitor to use.
 *  \param device_name     The name of the device of interest. */
void mmon_unsubscribe(mapper_monitor mon, const char *device_name);

/*! Interface to add a map between a set of signals.
 *  \param mon            The monitor to use for sending the message.
 *  \param num_sources    The number of source signals in this map.
 *  \param sources        Array of source signal data structures.
 *  \param dest           Destination signal data structure.
 *  \param properties     An optional data structure specifying the
 *                        requested properties of this map. The 'flags'
 *                        variable of this structure is used to indicate which
 *                        properties in the provided mapper_db_map_t
 *                        should be applied to the map. See the flags
 *                        prefixed by MAP_ in mapper_db.h. */
void mmon_map_signals_by_db_record(mapper_monitor mon, int num_sources,
                                   mapper_db_signal_t **sources,
                                   mapper_db_signal_t *dest,
                                   mapper_db_map_t *properties);

/*! Interface to add a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param num_sources  The number of source signals in this map.
 *  \param sources      Array of source signal names.
 *  \param dest         Destination signal name.
 *  \param properties   An optional data structure specifying the requested
 *                      properties of this map. The 'flags' variable of this
 *                      structure is used to indicate which properties in the
 *                      provided mapper_db_map_t should be applied to the map.
 *                      See the flags prefixed by MAP_ in mapper_db.h. */
void mmon_map_signals_by_name(mapper_monitor mon, int num_sources,
                              const char **sources, const char *dest,
                              mapper_db_map_t *properties);

/*! Interface to modify a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param map          A modified data structure specifying the map properties.
 *                      The 'flags' variable of this structure is used to
 *                      indicate which properties in the provided
 *                      mapper_db_map_t should be applied to the map.
 *                      See the flags prefixed by MAP_ in mapper_db.h. */
void mmon_modify_map(mapper_monitor mon, mapper_db_map_t *map);

/*! Interface to modify a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param num_sources  The number of source signals in this map.
 *  \param source       Array of source signal names.
 *  \param dest         Destination name.
 *  \param properties   An optional data structure specifying the requested
 *                      properties of this map. The 'flags' variable of this
 *                      structure is used to indicate which properties in the
 *                      provided mapper_db_map_t should be applied to the map.
 *                      See the flags prefixed by MAP_ in mapper_db.h. */
void mmon_modify_map_by_signal_names(mapper_monitor mon, int num_sources,
                                     const char **sources, const char *dest,
                                     mapper_db_map_t *properties);

/*! Interface to modify a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param num_sources  The number of source signals in this map.
 *  \param sources      Array of source signal data structures.
 *  \param dest         Destination signal data structure.
 *  \param properties   An optional data structure specifying the requested
 *                      properties of this map. The 'flags' variable of this
 *                      structure is used to indicate which properties in the
 *                      provided mapper_db_map_t should be applied to the map.
 *                      See the flags prefixed by MAP_ in mapper_db.h. */
void mmon_modify_map_by_signal_db_records(mapper_monitor mon, int num_sources,
                                          mapper_db_signal_t **sources,
                                          mapper_db_signal_t *dest,
                                          mapper_db_map_t *properties);

/*! Interface to remove a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param map          Map data structure. */
void mmon_remove_map(mapper_monitor mon, mapper_db_map_t *map);

/*! Interface to remove a map between a set of signals.
 *  \param mon          The monitor to use for sending the message.
 *  \param num_sources  The number of source signals in this map.
 *  \param sources      Array of source signal names.
 *  \param dest         Destination signal name. */
void mmon_unmap_signals_by_name(mapper_monitor mon, int num_sources,
                                const char **sources, const char *dest);

/*! Interface to remove a map between a set of signals.
 *  \param mon      The monitor to use for sending the message.
 *  \param num_sources    The number of source signals in this map.
 *  \param sources  Array of source signal data structures.
 *  \param dest     Destination signal data structure. */
void mmon_unmap_signals_by_db_record(mapper_monitor mon, int num_sources,
                                     mapper_db_signal_t **sources,
                                     mapper_db_signal_t *dest);

/*! Interface to send an arbitrary OSC message to the administrative bus.
 *  \param mon      The monitor to use for sending the message.
 *  \param path     The path for the OSC message.
 *  \param types    A string specifying the types of subsequent arguments.
 *  \param ...      A list of arguments. */
void mmon_send(mapper_monitor mon, const char *path, const char *types, ...);

/* @} */

/***** Time *****/

/*! @defgroup time Time

 @{ libmapper primarily uses NTP timetags for communication and
    synchronization. */

/*! Initialize a timetag to the current mapping network time.
 *  \param dev  The device whose time we are asking for.
 *  \param tt   A previously allocated timetag to initialize. */
void mdev_now(mapper_device dev, mapper_timetag_t *tt);

/*! Initialize a timetag to the current mapping network time.
 *  \param dev  The device whose time we are asking for.
 *  \param tt   A previously allocated timetag to initialize. */
void mmon_now(mapper_monitor mon, mapper_timetag_t *tt);

/*! Return the difference in seconds between two mapper_timetags.
 *  \param a    The minuend.
 *  \param b    The subtrahend.
 *  \return     The difference a-b in seconds. */
double mapper_timetag_difference(mapper_timetag_t a, mapper_timetag_t b);

/*! Add a timetag to another given timetag.
 *  \param timetag  A previously allocated timetag to augment.
 *  \param addend   A timetag to add to add. */
void mapper_timetag_add(mapper_timetag_t *tt, mapper_timetag_t addend);

/*! Subtract a timetag from another given timetag.
 *  \param timetag  A previously allocated timetag to augment.
 *  \param addend   A timetag to add to subtract. */
void mapper_timetag_subtract(mapper_timetag_t *tt, mapper_timetag_t addend);

/*! Add seconds to a given timetag.
 *  \param timetag  A previously allocated timetag to augment.
 *  \param addend   An amount in seconds to add. */
void mapper_timetag_add_double(mapper_timetag_t *tt, double addend);

/*! Return value of mapper_timetag as a double-precision floating point value. */
double mapper_timetag_get_double(mapper_timetag_t tt);

/*! Set value of a mapper_timetag from an integer value. */
void mapper_timetag_set_int(mapper_timetag_t *tt, int value);

/*! Set value of a mapper_timetag from a floating point value. */
void mapper_timetag_set_float(mapper_timetag_t *tt, float value);

/*! Set value of a mapper_timetag from a double-precision floating point value. */
void mapper_timetag_set_double(mapper_timetag_t *tt, double value);

/*! Copy value of a mapper_timetag. */
void mapper_timetag_cpy(mapper_timetag_t *ttl, mapper_timetag_t ttr);

/* @} */

#ifdef __cplusplus
}
#endif

#endif // __MAPPER_H__
