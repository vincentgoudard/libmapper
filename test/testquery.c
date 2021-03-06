#include "../src/mapper_internal.h"
#include <mapper/mapper.h>
#include <stdio.h>
#include <math.h>
#include <lo/lo.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#endif

#define eprintf(format, ...) do {               \
    if (verbose)                                \
        fprintf(stdout, format, ##__VA_ARGS__); \
} while(0)

int verbose = 1;
int terminate = 0;
int autoconnect = 1;

mapper_device source = 0;
mapper_device destination = 0;
mapper_signal sendsig[4] = {0, 0, 0, 0};
mapper_signal recvsig[4] = {0, 0, 0, 0};

int sent = 0;
int received = 0;
int done = 0;

void query_response_handler(mapper_signal sig, mapper_id instance,
                            const void *value, int count,
                            mapper_timetag_t *timetag)
{
    int i;
    if (value) {
        eprintf("--> source got query response: %s ", mapper_signal_name(sig));
        for (i = 0; i < mapper_signal_length(sig) * count; i++)
            eprintf("%i ", ((int*)value)[i]);
        eprintf("\n");
    }
    else {
        eprintf("--> source got empty query response: %s\n",
                mapper_signal_name(sig));
    }

    received++;
}

/*! Creation of a local source. */
int setup_source()
{
    char sig_name[20];
    source = mapper_device_new("testquery-send", 0, 0);
    if (!source)
        goto error;
    eprintf("source created.\n");

    int mn[]={0,0,0,0}, mx[]={10,10,10,10};

    for (int i = 0; i < 4; i++) {
        snprintf(sig_name, 20, "%s%i", "outsig_", i);
        sendsig[i] = mapper_device_add_output_signal(source, sig_name, i+1, 'i',
                                                     0, mn, mx);
        mapper_signal_set_callback(sendsig[i], query_response_handler);
        mapper_signal_update(sendsig[i], mn, 0, MAPPER_NOW);
    }

    eprintf("Output signals registered.\n");
    eprintf("Number of outputs: %d\n",
            mapper_device_num_signals(source, MAPPER_DIR_OUTGOING));

    return 0;

error:
    return 1;
}

void cleanup_source()
{
    if (source) {
        eprintf("Freeing source.. ");
        fflush(stdout);
        mapper_device_free(source);
        eprintf("ok\n");
    }
}

void insig_handler(mapper_signal sig, mapper_id instance, const void *value,
                   int count, mapper_timetag_t *timetag)
{
    if (value) {
        eprintf("--> destination got %s %f\n", mapper_signal_name(sig),
                (*(float*)value));
    }
    received++;
}

/*! Creation of a local destination. */
int setup_destination()
{
    char sig_name[10];
    destination = mapper_device_new("testquery-recv", 0, 0);
    if (!destination)
        goto error;
    eprintf("destination created.\n");

    float mn[]={0,0,0,0}, mx[]={1,1,1,1};

    for (int i = 0; i < 4; i++) {
        snprintf(sig_name, 10, "%s%i", "insig_", i);
        recvsig[i] = mapper_device_add_input_signal(destination, sig_name, i+1,
                                                    'f', 0, mn, mx,
                                                    insig_handler, 0);
    }

    eprintf("Input signal 'insig' registered.\n");
    eprintf("Number of inputs: %d\n",
            mapper_device_num_signals(destination, MAPPER_DIR_INCOMING));

    return 0;

error:
    return 1;
}

void cleanup_destination()
{
    if (destination) {
        eprintf("Freeing destination.. ");
        fflush(stdout);
        mapper_device_free(destination);
        eprintf("ok\n");
    }
}

void wait_local_devices()
{
    while (!done && !(mapper_device_ready(source)
                      && mapper_device_ready(destination))) {
        mapper_device_poll(source, 0);
        mapper_device_poll(destination, 0);

        usleep(50 * 1000);
    }
}

int setup_maps()
{
    int i;
    mapper_map maps[4] = {0, 0, 0, 0};
    for (i = 0; i < 2; i++) {
        maps[i] = mapper_map_new(1, &sendsig[i], 1, &recvsig[i]);
        mapper_map_push(maps[i]);
    }

    // swap the last two signals to mix up signal vector lengths
    maps[2] = mapper_map_new(1, &sendsig[2], 1, &recvsig[3]);
    mapper_map_push(maps[2]);
    maps[3] = mapper_map_new(1, &sendsig[3], 1, &recvsig[2]);
    mapper_map_push(maps[3]);

    i = 0;
    int ready = 0;
    // wait until mapping has been established
    while (!done && !ready) {
        mapper_device_poll(source, 10);
        mapper_device_poll(destination, 10);
        if (i++ > 100)
            return 1;
        ready = 1;
        for (i = 0; i < 4; i++) {
            if (!mapper_map_ready(maps[i])) {
                ready = 0;
                break;
            }
        }
    }

    return 0;
}

void loop()
{
    eprintf("-------------------- GO ! --------------------\n");
    int i = 10, j = 0, count;
    float value[] = {0., 0., 0., 0.};

    while ((!terminate || i < 50) && !done) {
        for (j = 0; j < 4; j++)
            value[j] = (i % 10) * 1.0f;
        for (j = 0; j < 4; j++) {
            mapper_signal_update(recvsig[j], value, 0, MAPPER_NOW);
        }
        eprintf("\ndestination values updated to %f -->\n", (i % 10) * 1.0f);
        for (j = 0; j < 4; j++) {
            sent += count = mapper_signal_query_remotes(sendsig[j], MAPPER_NOW);
            eprintf("Sent %i queries for sendsig[%i]\n", count, j);
        }
        mapper_device_poll(destination, 50);
        mapper_device_poll(source, 50);
        i++;

        if (!verbose) {
            printf("\r  Sent: %4i, Received: %4i   ", sent, received);
            fflush(stdout);
        }
    }
}

void ctrlc(int sig)
{
    done = 1;
}

int main(int argc, char **argv)
{
    int i, j, result = 0;

    // process flags for -v verbose, -t terminate, -h help
    for (i = 1; i < argc; i++) {
        if (argv[i] && argv[i][0] == '-') {
            int len = strlen(argv[i]);
            for (j = 1; j < len; j++) {
                switch (argv[i][j]) {
                    case 'h':
                        eprintf("testquery.c: possible arguments "
                                "-q quiet (suppress output), "
                                "-t terminate automatically, "
                                "-h help\n");
                        return 1;
                        break;
                    case 'q':
                        verbose = 0;
                        break;
                    case 't':
                        terminate = 1;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    signal(SIGINT, ctrlc);

    if (setup_destination()) {
        eprintf("Error initializing destination.\n");
        result = 1;
        goto done;
    }

    if (setup_source()) {
        eprintf("Done initializing source.\n");
        result = 1;
        goto done;
    }

    wait_local_devices();

    if (autoconnect && setup_maps()) {
        eprintf("Error connecting signals.\n");
        result = 1;
        goto done;
    }

    loop();

    if (sent != received) {
        eprintf("Not all sent queries received responses.\n");
        eprintf("Queried %d time%s, but received %d responses.\n",
                sent, sent == 1 ? "" : "s", received);
        result = 1;
    }

done:
    cleanup_destination();
    cleanup_source();
    printf("Test %s.\n", result ? "FAILED" : "PASSED");
    return result;
}
