/*
 * OPC UA TSN configuration recommendations
 * Compile:
 * gcc -std=c11 -Wall -Wextra -O2 PubSubGenerator.c -o PubSubGenerator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * -------------------------
 * Constants
 * ------------------------- 
 */


#define MAX_LINE 256
#define MAX_FLOWS 128

/*
 * -------------------------
 * Enums
 * ------------------------- 
 */

typedef enum {
    TSN_ST = 0,
    TSN_AVB,
    TSN_BE
} TSNClass;

typedef enum {
    TRAFFIC_CONTROL_ISO = 0,
    TRAFFIC_CONTROL_SYNC,
    TRAFFIC_CONTROL_ASYNC,
    TRAFFIC_EVENT,
    TRAFFIC_VOICE_VIDEO,
    TRAFFIC_NETWORK,
    TRAFFIC_COMMAND_CYCLE,
    TRAFFIC_COMMAND_ACYCLE,
    TRAFFIC_CONFIG,
    TRAFFIC_DIAGNOSTIC_CYCLE,
    TRAFFIC_DIAGNOSTIC_ACYCLE,
    TRAFFIC_BEST_EFFORT,
    TRAFFIC_DEFAULT,
    TRAFFIC_ID_COUNT
} TrafficID;

typedef enum {
    DSM_KEY_FRAME = 0,
    DSM_DELTA_FRAME,
    DSM_EVENT,
    DSM_CHUNK_MESSAGE
} DSMType;

typedef enum {
    DSO_UNDEFINED = 0,
    DSO_ASCENDING_WRITER_ID,
    DSO_ASCENDING_WRITER_ID_SINGLE
} DatasetOrdering;

typedef enum {
    ENCODING_UADP = 0,
    ENCODING_JSON
} EncodingMimeType;

typedef enum {
    TP_UDP_UADP = 0,
    TP_BROKER_JSON
} TransportProfileUri;

/* -------------------------
 * Structs
 * ------------------------- */

typedef struct {
    TrafficID traffic_id;
    const char *application_class;
    const char *traffic_type;
    const char *Synchronization;
    const char *Periodicity;
    const char *cycle_time;
    const char *transmission_guarantee;
    const char *latency;
    const char *criticality;
    const char *jitter_tolerance;
    const char *loss_tolerance;
    const char *length_variability;
    const char *length;
    int  has_deadline_or_bounded_latency;      
    int  low_jitter;
    int  high_critical;
} TrafficSpec;

typedef struct {
    TSNClass primary;
    int has_alternative;
    TSNClass alternative;

    int P;
    int D;
    int JO;
    int HRT;
} TSNDecision;

typedef struct {
    int cyclic_dataset;

    DSMType dsm_type[4];
    size_t dsm_type_count;

    int enable_delta_frames;

    int key_frame_count_value;         
    const char *key_frame_count_text;  

    const char *publishing_interval;
    const char *keep_alive_time_hint;
    int keepalive_count;

    DatasetOrdering dataset_ordering[4];
    size_t dataset_ordering_count;

    EncodingMimeType encoding_mime_type[2];
    size_t encoding_mime_type_count;

    TransportProfileUri transport_profile_uri[2];
    size_t transport_profile_uri_count;
} PubSubRecommendation;

typedef struct
{
    char flow_id[32];
    int periodic;                 
    int traffic_application_class;
} Config;

/* -------------------------
 * String helpers
 * ------------------------- */

static const char *tsnclass_to_string(TSNClass c) {
    switch (c) {
        case TSN_ST:  return "ST";
        case TSN_AVB: return "AVB";
        case TSN_BE:  return "BE";
        default:      return "Unknown";
    }
}

static const char *trafficid_to_string(TrafficID id) {
    switch (id) {
        case TRAFFIC_CONTROL_ISO:        return "Control-Iso";
        case TRAFFIC_CONTROL_SYNC:       return "Control-Sync";
        case TRAFFIC_CONTROL_ASYNC:      return "Control-Async";
        case TRAFFIC_EVENT:              return "Event";
        case TRAFFIC_VOICE_VIDEO:        return "Voice/Video";
        case TRAFFIC_NETWORK:            return "Network";
        case TRAFFIC_COMMAND_CYCLE:      return "Command-Cycle";
        case TRAFFIC_COMMAND_ACYCLE:     return "Command-Acycle";
        case TRAFFIC_CONFIG:             return "Config";
        case TRAFFIC_DIAGNOSTIC_CYCLE:   return "Diagnostic-Cycle";
        case TRAFFIC_DIAGNOSTIC_ACYCLE:  return "Diagnostic-Acycle";
        case TRAFFIC_BEST_EFFORT:        return "Best Effort";
        case TRAFFIC_DEFAULT:            return "Default";
        default:                         return "Unknown";
    }
}

static const char *dsmtype_to_string(DSMType t) {
    switch (t) {
        case DSM_KEY_FRAME:     return "KeyFrame";
        case DSM_DELTA_FRAME:   return "DeltaFrame";
        case DSM_EVENT:         return "Event";
        case DSM_CHUNK_MESSAGE: return "Chunk message";
        default:                return "Unknown";
    }
}

static const char *datasetordering_to_string(DatasetOrdering d) {
    switch (d) {
        case DSO_UNDEFINED:                  return "Undefined";
        case DSO_ASCENDING_WRITER_ID:        return "AscendingWriterID";
        case DSO_ASCENDING_WRITER_ID_SINGLE: return "AscendingWriterIDSingle";
        default:                             return "Unknown";
    }
}

static const char *encoding_to_string(EncodingMimeType e) {
    switch (e) {
        case ENCODING_UADP: return "UA_PUBSUB_ENCODING_UADP";
        case ENCODING_JSON: return "UA_PUBSUB_ENCODING_JSON";
        default:            return "Unknown";
    }
}

static const char *transport_to_string(TransportProfileUri t) {
    switch (t) {
        case TP_UDP_UADP:     return "UDP UADP";
        case TP_BROKER_JSON:  return "JSON";
        default:              return "Unknown";
    }
}

/* -------------------------
 * Static TABLE[TrafficID]
 * ------------------------- */

static const TrafficSpec TABLE[TRAFFIC_ID_COUNT] = {
    [TRAFFIC_CONTROL_ISO] = {
        .traffic_id = TRAFFIC_CONTROL_ISO,
        .application_class = "Open/Closed (Distributed) Control Loop and Motion Control",
        .traffic_type = "Isochronous",
        .Synchronization = "Synchronous",
        .Periodicity = "Periodic",
        .cycle_time = "µs–10s of ms",
        .transmission_guarantee = "Deadline",
        .latency = "< T",
        .criticality = "High",
        .jitter_tolerance = "0",
        .loss_tolerance = "No",
        .length_variability = "Fixed",
        .length = "30–500",
        .low_jitter = 1,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 1
    },
    [TRAFFIC_CONTROL_SYNC] = {
        .traffic_id = TRAFFIC_CONTROL_SYNC,
        .application_class = "Open/Closed (Distributed) Control Loop and Motion Control",
        .traffic_type = "Cyclic-Synchronous",
        .Synchronization = "Synchronous",
        .Periodicity = "Periodic",
        .cycle_time = "100s of µs–100s of ms",
        .transmission_guarantee = "Frame Latency",
        .latency = "< T",
        .criticality = "High",
        .jitter_tolerance = "< L",
        .loss_tolerance = "No",
        .length_variability = "Fixed",
        .length = "Unconstrained",
        .low_jitter = 1,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 1
    },
    [TRAFFIC_CONTROL_ASYNC] = {
        .traffic_id = TRAFFIC_CONTROL_ASYNC,
        .application_class = "Factory Automation and Non Synchronized Control",
        .traffic_type = "Cyclic-Asynchronous",
        .Synchronization = "Asynchronous",
        .Periodicity = "Periodic",
        .cycle_time = "ms – s",
        .transmission_guarantee = "Frame Latency",
        .latency = "< 90% * T",
        .criticality = "High",
        .jitter_tolerance = "< L",
        .loss_tolerance = "1–4 Frames",
        .length_variability = "Fixed",
        .length = "Unconstrained",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 1
    },
    [TRAFFIC_EVENT] = {
        .traffic_id = TRAFFIC_EVENT,
        .application_class = "Control Events and Alarms",
        .traffic_type = "Acyclic",
        .Synchronization = "Asynchronous",
        .Periodicity = "Aperiodic",
        .cycle_time = "N/A",
        .transmission_guarantee = "Flow Latency",
        .latency = "10–50 ms",
        .criticality = "High",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (100–200)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 1
    },
    [TRAFFIC_VOICE_VIDEO] = {
        .traffic_id = TRAFFIC_VOICE_VIDEO,
        .application_class = "Voice/Video",
        .traffic_type = "Cyclic-Asynchronous",
        .Synchronization = "Asynchronous",
        .Periodicity = "Periodic",
        .cycle_time = "Sample Time/Frame Rate",
        .transmission_guarantee = "Flow Latency",
        .latency = "10 ms/100 ms",
        .criticality = "Low",
        .jitter_tolerance = "< L",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "1000–1500",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_NETWORK] = {
        .traffic_id = TRAFFIC_NETWORK,
        .application_class = "Network Control and Inter-network Control",
        .traffic_type = "Cyclic-Asynchronous",
        .Synchronization = "Asynchronous",
        .Periodicity = "Periodic",
        .cycle_time = "50 ms – 1 s",
        .transmission_guarantee = "Flow Latency",
        .latency = "-",
        .criticality = "High",
        .jitter_tolerance = "< L",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (50–500)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 0,
        .high_critical = 1
    },
    [TRAFFIC_COMMAND_CYCLE] = {
        .traffic_id = TRAFFIC_COMMAND_CYCLE,
        .application_class = "Operator Commands and HMI Interactions",
        .traffic_type = "Cyclic-Asynchronous",
        .Synchronization = "Asynchronous",
        .Periodicity = "Periodic",
        .cycle_time = "-",
        .transmission_guarantee = "Flow Latency",
        .latency = "< 2 s",
        .criticality = "Medium",
        .jitter_tolerance = "Yes",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "100–1500",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_COMMAND_ACYCLE] = {
        .traffic_id = TRAFFIC_COMMAND_ACYCLE,
        .application_class = "Operator Commands and HMI Interactions",
        .traffic_type = "Acyclic",
        .Synchronization = "Asynchronous",
        .Periodicity = "Aperiodic",
        .cycle_time = "N/A",
        .transmission_guarantee = "Flow Latency",
        .latency = "< 2 s",
        .criticality = "Medium",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "100–1500",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_CONFIG] = {
        .traffic_id = TRAFFIC_CONFIG,
        .application_class = "Configuration and Management",
        .traffic_type = "Acyclic",
        .Synchronization = "Asynchronous",
        .Periodicity = "Aperiodic",
        .cycle_time = "N/A",
        .transmission_guarantee = "Flow Latency",
        .latency = "Up to Seconds",
        .criticality = "Medium",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (500–1500)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_DIAGNOSTIC_CYCLE] = {
        .traffic_id = TRAFFIC_DIAGNOSTIC_CYCLE,
        .application_class = "Diagnostics and Monitoring",
        .traffic_type = "Cyclic-Asynchronous",
        .Synchronization = "Asynchronous",
        .Periodicity = "Periodic",
        .cycle_time = "ms – s",
        .transmission_guarantee = "Flow Latency",
        .latency = "100 ms",
        .criticality = "Medium",
        .jitter_tolerance = "Yes",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (500–1500)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_DIAGNOSTIC_ACYCLE] = {
        .traffic_id = TRAFFIC_DIAGNOSTIC_ACYCLE,
        .application_class = "Diagnostics and Monitoring",
        .traffic_type = "Acyclic",
        .Synchronization = "Asynchronous",
        .Periodicity = "Aperiodic",
        .cycle_time = "N/A",
        .transmission_guarantee = "Flow Latency",
        .latency = "100 ms",
        .criticality = "Medium",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (500–1500)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 1,
        .high_critical = 0
    },
    [TRAFFIC_BEST_EFFORT] = {
        .traffic_id = TRAFFIC_BEST_EFFORT,
        .application_class = "Best Effort",
        .traffic_type = "Acyclic",
        .Synchronization = "Asynchronous",
        .Periodicity = "Aperiodic",
        .cycle_time = "N/A",
        .transmission_guarantee = "NO",
        .latency = "N/A",
        .criticality = "Low",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "Yes",
        .length_variability = "Variable",
        .length = "Unconstrained (30–1500)",
        .low_jitter = 0,
        .has_deadline_or_bounded_latency = 0,
        .high_critical = 0
    }, 
    [TRAFFIC_DEFAULT] = {
        .traffic_id = TRAFFIC_DEFAULT,
        .application_class = "TRAFFIC_DEFAULT",
        .traffic_type = "N/A",
        .Synchronization = "N/A",
        .Periodicity = "N/A",
        .cycle_time = "N/A",
        .transmission_guarantee = "N/A",
        .latency = "N/A",
        .criticality = "N/A",
        .jitter_tolerance = "N/A",
        .loss_tolerance = "N/A",
        .length_variability = "N/A",
        .length = "N/A"
    }
};

/* -------------------------
 * Input helpers
 * ------------------------- */

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void trim(char *s)
{
    char *p = s;

    // left trim
    while (*p == ' ' || *p == '\t') p++;
    memmove(s, p, strlen(p) + 1);

    // right trim
    for (int i = strlen(s) - 1; i >= 0 &&
         (s[i] == '\n' || s[i] == '\r' || s[i] == ' ' || s[i] == '\t'); i--)
        s[i] = 0;
}

static int ask_choice(const char *prompt, const char *options[], size_t option_count) {
    char line[MAX_LINE];

    for (;;) {
        printf("\n%s\n", prompt);
        for (size_t i = 0; i < option_count; i++) {
            printf("  %zu. %s\n", i + 1, options[i]);
        }
        printf("Select an option (number): ");
        if (!fgets(line, sizeof(line), stdin)) {
            return 1;
        }
        trim_newline(line);

        int ok = 1;
        if (line[0] == '\0') ok = 0;
        for (size_t i = 0; line[i] != '\0'; i++) {
            if (!isdigit((unsigned char)line[i])) {
                ok = 0;
                break;
            }
        }
        if (!ok) {
            printf("Invalid input. Enter a valid option number.\n");
            continue;
        }

        long idx = strtol(line, NULL, 10);
        if (idx >= 1 && (size_t)idx <= option_count) {
            return (int)idx;
        }
        printf("Invalid input. Enter a valid option number.\n");
    }
}

void get_string(char *buffer, size_t size, const char *prompt) {
    printf("%s", prompt);
    if (!fgets(buffer, size, stdin)) {
        buffer[0] = '\0';
        return;
    }
    trim_newline(buffer);
}

static int ask_yes_no(const char *prompt) {
    const char *opts[] = {"Yes", "No"};
    return ask_choice(prompt, opts, 2) == 1;
}

int load_config_ini(const char *filename, Config *cfg)
{
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[512];

    while (fgets(line, sizeof(line), f))
    {
        trim(line);
        // skip empty or comments
        if (line[0] == '#' || line[0] == ';' || line[0] == '\0' || line[0] == '[')
            continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = 0;
        char *key = line;
        char *value = eq + 1;

        trim(key);
        trim(value);

        if (strcmp(key, "periodic") == 0)
            cfg->periodic = atoi(value);

        else if (strcmp(key, "traffic_application_class") == 0)
            cfg->traffic_application_class = atoi(value);       
    }
    fclose(f);
    return 0;
}


static int load_bulk_config_ini(const char *filename, Config flows[], int max_flows) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[512];
    int count = 0;
    int current = -1;

    while (fgets(line, sizeof(line), f)) {
        trim(line);

        if (line[0] == '#' || line[0] == ';' || line[0] == '\0')
            continue;

        if (line[0] == '[') {
            if (count >= max_flows) {
                fclose(f);
                return count;
            }
            current = count++;
            memset(&flows[current], 0, sizeof(Config));
            continue;
        }

        if (current < 0)
            continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *value = eq + 1;

        trim(key);
        trim(value);

        if (strcmp(key, "flow_id") == 0) {
            strncpy(flows[current].flow_id, value, sizeof(flows[current].flow_id) - 1);
        } else if (strcmp(key, "periodic") == 0) {
            flows[current].periodic = atoi(value);
        } else if (strcmp(key, "traffic_application_class") == 0) {
            flows[current].traffic_application_class = atoi(value);
        }
    }

    fclose(f);
    return count;
}

/* -------------------------
 * Core logic
 * ------------------------- */

static TSNDecision classify_tsn(int periodic, int has_deadline_or_bounded_latency, int low_jitter, int high_critical) {
    TSNDecision dec;
    dec.P = periodic ? 1 : 0;
    dec.D = has_deadline_or_bounded_latency ? 1 : 0;
    dec.JO = low_jitter ? 1 : 0;
    dec.HRT = high_critical ? 1 : 0;

    int st  = (dec.P && (dec.JO || dec.D)) ? 1 : 0;
    int avb = (dec.D && !(dec.JO && dec.HRT)) ? 1 : 0;

    if (st && avb) {
        dec.primary = TSN_AVB;
        dec.has_alternative = 1;
        dec.alternative = TSN_ST;
        return dec;
    }
    if (st) {
        dec.primary = TSN_ST;
        dec.has_alternative = 0;
        return dec;
    }
    if (avb) {
        dec.primary = TSN_AVB;
        dec.has_alternative = 0;
        return dec;
    }
    dec.primary = TSN_BE;
    dec.has_alternative = 0;
    return dec;
}

// static TrafficID infer_traffic_id(int periodic, int has_deadline_or_bounded_latency, int low_jitter, int high_critical, int usecase, int synchronization) {
//     int D = has_deadline_or_bounded_latency ? 1 : 0;
//     /* Aperiodic */
//     if (!periodic) {
//         if (high_critical) {
//             return TRAFFIC_EVENT;
//         }
//         if (!D) {
//             return TRAFFIC_BEST_EFFORT;
//         }
//         if (usecase == 1) return TRAFFIC_DIAGNOSTIC_ACYCLE;
//         if (usecase == 2) return TRAFFIC_COMMAND_ACYCLE;
//         if (usecase == 0) return TRAFFIC_CONFIG;
//         return TRAFFIC_DEFAULT;
//     }

//     /* Periodic */
//     if (high_critical) {
//         if (D && (low_jitter == 1) && synchronization == 1) {
//             return TRAFFIC_CONTROL_SYNC;                
//         }
//         if (D && (low_jitter == 2))
//         {
//             return TRAFFIC_CONTROL_ISO;
//         }
//         if(D && (low_jitter == 0 || low_jitter == 1) && synchronization == 0 ){
//             return TRAFFIC_CONTROL_ASYNC;
//         }
//         if (!D) {
//             return TRAFFIC_NETWORK;
//         }
//     }
//     else
//     {
//     if (usecase == 3) return TRAFFIC_VOICE_VIDEO;        
//     if (usecase == 2) return TRAFFIC_COMMAND_CYCLE;
//     if (usecase == 1) return TRAFFIC_DIAGNOSTIC_CYCLE;        
//     }
//     return TRAFFIC_DEFAULT;
// }

static TrafficID infer_traffic_id(int periodic, int traffic_application_class) {
    switch (traffic_application_class) {
        case 1:
            return TRAFFIC_CONTROL_ISO;

        case 2:
            return TRAFFIC_CONTROL_SYNC;

        case 3:
            return TRAFFIC_CONTROL_ASYNC;

        case 4:
            return TRAFFIC_EVENT;

        case 5:
            return TRAFFIC_VOICE_VIDEO;

        case 6:
            return TRAFFIC_NETWORK;

        case 7:
            return periodic ? TRAFFIC_COMMAND_CYCLE : TRAFFIC_COMMAND_ACYCLE;

        case 8:
            return TRAFFIC_CONFIG;

        case 9:
            return periodic ? TRAFFIC_DIAGNOSTIC_CYCLE : TRAFFIC_DIAGNOSTIC_ACYCLE;

        case 10:
            return TRAFFIC_BEST_EFFORT;
        default: 
            return TRAFFIC_DEFAULT;
    }
}

static PubSubRecommendation recommend_pubsub_config(TrafficID traffic_id, const TrafficSpec *spec) {
    PubSubRecommendation rec;
    memset(&rec, 0, sizeof(rec));

    rec.key_frame_count_value = 1;
    rec.key_frame_count_text = "1";
    rec.keep_alive_time_hint = NULL;

    if (traffic_id == TRAFFIC_CONTROL_ISO || traffic_id == TRAFFIC_CONTROL_SYNC || traffic_id == TRAFFIC_CONTROL_ASYNC) {
        rec.cyclic_dataset = 1;

        rec.dsm_type[0] = DSM_KEY_FRAME;
        rec.dsm_type_count = 1;

        rec.enable_delta_frames = 0;
        rec.key_frame_count_value = 1;
        rec.key_frame_count_text = "1";

        rec.keep_alive_time_hint = "Definition is not needed";
        rec.keepalive_count = 0;

        rec.dataset_ordering[0] = DSO_ASCENDING_WRITER_ID_SINGLE;
        rec.dataset_ordering[1] = DSO_ASCENDING_WRITER_ID;
        rec.dataset_ordering_count = 2;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type_count = 1;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri_count = 1;

        rec.publishing_interval = spec->cycle_time;
    }
    else if (traffic_id == TRAFFIC_EVENT) {
        rec.cyclic_dataset = 0;

        rec.dsm_type[0] = DSM_EVENT;
        rec.dsm_type_count = 1;

        rec.enable_delta_frames = 0;

        rec.keep_alive_time_hint = "Should be defined in relation to the use case";
        rec.keepalive_count = 1;

        rec.dataset_ordering[0] = DSO_ASCENDING_WRITER_ID_SINGLE;
        rec.dataset_ordering[1] = DSO_UNDEFINED;
        rec.dataset_ordering_count = 2;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type_count = 1;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri_count = 1;

        rec.publishing_interval = spec->cycle_time;
    }
    else if (traffic_id == TRAFFIC_VOICE_VIDEO) {
        rec.cyclic_dataset = 1;

        rec.dsm_type[0] = DSM_CHUNK_MESSAGE;
        rec.dsm_type_count = 1;

        rec.enable_delta_frames = 0;

        rec.keep_alive_time_hint = "Definition is not needed";
        rec.keepalive_count = 0;

        rec.dataset_ordering[0] = DSO_UNDEFINED;
        rec.dataset_ordering_count = 1;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type_count = 1;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri_count = 1;

        rec.publishing_interval = spec->cycle_time;
    }
    else if (traffic_id == TRAFFIC_COMMAND_CYCLE) {
        rec.cyclic_dataset = 1;

        rec.dsm_type[0] = DSM_KEY_FRAME;
        rec.dsm_type[1] = DSM_DELTA_FRAME;
        rec.dsm_type_count = 2;

        rec.enable_delta_frames = 1;

        rec.key_frame_count_value = -1;
        rec.key_frame_count_text = "1 if DSM Type is KeyFrame, >1 if DSM Type is DeltaFrame";

        rec.keep_alive_time_hint = "Should be defined in relation to the use case if DeltaFrame is the choice";
        rec.keepalive_count = 1;

        rec.dataset_ordering[0] = DSO_UNDEFINED;
        rec.dataset_ordering_count = 1;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type[1] = ENCODING_JSON;
        rec.encoding_mime_type_count = 2;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri[1] = TP_BROKER_JSON;
        rec.transport_profile_uri_count = 2;

        rec.publishing_interval = spec->cycle_time;
    }
    else if (traffic_id == TRAFFIC_COMMAND_ACYCLE || traffic_id == TRAFFIC_CONFIG ||
             traffic_id == TRAFFIC_DIAGNOSTIC_ACYCLE || traffic_id == TRAFFIC_BEST_EFFORT) {

        rec.cyclic_dataset = 0;

        rec.dsm_type[0] = DSM_EVENT;
        rec.dsm_type_count = 1;

        rec.enable_delta_frames = 0;
        rec.key_frame_count_value = 1;
        rec.key_frame_count_text = "1";

        rec.keep_alive_time_hint = "Should be defined in relation to the use case";
        rec.keepalive_count = 1;

        rec.dataset_ordering[0] = DSO_UNDEFINED;
        rec.dataset_ordering_count = 1;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type[1] = ENCODING_JSON;
        rec.encoding_mime_type_count = 2;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri[1] = TP_BROKER_JSON;
        rec.transport_profile_uri_count = 2;

        rec.publishing_interval = spec->cycle_time;
    }
    else if (traffic_id == TRAFFIC_DIAGNOSTIC_CYCLE) {
        rec.cyclic_dataset = 1;

        rec.dsm_type[0] = DSM_KEY_FRAME;
        rec.dsm_type[1] = DSM_DELTA_FRAME;
        rec.dsm_type_count = 2;

        rec.enable_delta_frames = 1;

        rec.key_frame_count_value = -1;
        rec.key_frame_count_text = "1 if DSM Type is KeyFrame, >1 if DSM Type is DeltaFrame";

        rec.keep_alive_time_hint = "Should be defined in relation to the use case if DeltaFrame is the choice";
        rec.keepalive_count = 1;

        rec.dataset_ordering[0] = DSO_UNDEFINED;
        rec.dataset_ordering_count = 1;

        rec.encoding_mime_type[0] = ENCODING_UADP;
        rec.encoding_mime_type[1] = ENCODING_JSON;
        rec.encoding_mime_type_count = 2;

        rec.transport_profile_uri[0] = TP_UDP_UADP;
        rec.transport_profile_uri[1] = TP_BROKER_JSON;
        rec.transport_profile_uri_count = 2;

        rec.publishing_interval = spec->cycle_time;
    }
    else {
        /* fallback */
        rec.cyclic_dataset = 0;
        rec.dsm_type[0] = DSM_KEY_FRAME;
        rec.dsm_type_count = 1;
        rec.enable_delta_frames = 0;
        rec.key_frame_count_value = 1;
        rec.key_frame_count_text = "1";
        rec.publishing_interval = spec->cycle_time;
    }

    return rec;
}

void write_reco_ini(const char *path,
                    TrafficID traffic_id,
                    const TSNDecision *tsn,
                    const PubSubRecommendation *pubrec
                    )
{
    int uadp = 1;
    int mqtt = 0;
    char transporturl[256];
    if (pubrec->transport_profile_uri_count == 2) {
        uadp = ask_yes_no("Use UADP (UDP) encoding? (No = JSON)");
        if (uadp == 1) {
            snprintf(transporturl, sizeof(transporturl), "opc.udp");
        } else {
            mqtt = ask_yes_no("Use JSON over MQTT? (No = AMQP)");
            snprintf(transporturl, sizeof(transporturl), mqtt == 1 ? "opc.mqtt" : "opc.amqp");
        }
    } else {
        uadp = 1;
        snprintf(transporturl, sizeof(transporturl), "opc.udp");
    }

    int delta = 0;
    char key_frame_count[256];
    int key_frame_count_int = 1;
    char keepalive_time[256];
    int keepalive_time_int = 0;
    if(pubrec->enable_delta_frames){
        delta = ask_yes_no("Are you going to use DeltaFrame?");
        if (delta == 1)
        {
            get_string(key_frame_count, sizeof(key_frame_count), "\nSpecify the number of key frames:\n");
            key_frame_count_int = atoi(key_frame_count);
            get_string(keepalive_time, sizeof(keepalive_time), "\nSpecify keepalive time for this traffic type:\n");
            keepalive_time_int = atoi(keepalive_time);
        }
    }

    if (!pubrec->cyclic_dataset)
    {
        get_string(keepalive_time, sizeof(keepalive_time), "\nSpecify keepalive time for this traffic type:\n");
        keepalive_time_int = atoi(keepalive_time); 
    }
 
    char connection_string[512];
    snprintf( connection_string, sizeof(connection_string), "%s:%s", transporturl, "!!!Specify network address e.g. \\\\127.0.0.1:1883");

    const char *published_dataset = (pubrec->cyclic_dataset) ? "UA_PUBSUB_DATASET_PUBLISHEDITEMS" : "UA_PUBSUB_DATASET_PUBLISHEDEVENTS";

    const char *transport_profile = uadp == 1 ? "UA_PUBSUB_TRANSPORTPROFILE_UDP_UADP" : (mqtt == 1 ? "http://opcfoundation.org/UA-Profile/Transport/pubsub-mqtt-json": "http://opcfoundation.org/UA-Profile/Transport/pubsub-amqp-json");
     
    const char *encode_type = uadp == 1 ? "UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE" : "UA_TYPES_JSONWRITERGROUPMESSAGEDATATYPE";

    int dataset_ordering = -1;
    if (pubrec->dataset_ordering_count > 1)
    {
        dataset_ordering = 1;
    }


    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f, ";This Configuration file considers having only one dataset in the network message. Consider adding more datasets if they exist.\n");

    /* ---- Traffic ---- */
    fprintf(f, "traffic_id=%s\n\n", trafficid_to_string(traffic_id));
    /* ---- TSN decision ---- */
    fprintf(f, "tsn.primary=%s\n", tsnclass_to_string(tsn->primary));
    if (tsn->has_alternative) {
        fprintf(f, "tsn.alternative=%s\n", tsnclass_to_string(tsn->alternative));
    }
    fprintf(f, "tsn.P=%d\n", tsn->P);
    fprintf(f, "tsn.D=%d\n", tsn->D);
    fprintf(f, "tsn.JO=%d\n", tsn->JO);
    fprintf(f, "tsn.HRT=%d\n\n", tsn->HRT);

    /* ---- PubSub recommendations ---- */

    fprintf(f,
    ";Consider changing all the Names and IDs before using the Publisher generator file. \n "
    ";In the generated publisher file consider defining the dataset fields. One String field is defined as an example.\n"
    ";The assumption in this configuration file is that the user opts UADP as the encoding for all traffic types.\n"
    ";Chunk messages are not yet implemented, thus the voice and video messages will be sent as normal dataset UADP messages.\n"
    );


    fprintf(f, "pubsub.connection_name=%s\n", "Connection_Name_Demo");
    fprintf(f, "pubsub.publisher_id=%d\n", 2525);
    fprintf(f, "pubsub.publishedDataSetConfig_name=%s\n", "PDS_Demo");
    fprintf(f, "pubsub.writerGroupId=%d\n", 100);
    fprintf(f, "pubsub.writerGroupConfig_name=%s\n", "WriterGroup_Demo");
    fprintf(f, "pubsub.dataSetWriterId=%d\n", 200);
    fprintf(f, "pubsub.dataSetWriterName=%s\n\n", "dataSetWriter_Demo");

    fprintf(f, "pubsub.transportProfile=%s\n", transport_profile);
    fprintf(f, "pubsub.encodingMimeType=%s\n", uadp == 1 ? "UA_PUBSUB_ENCODING_UADP": "UA_PUBSUB_ENCODING_JSON");
    fprintf(f, "pubsub.cyclic_dataset=%d\n", pubrec->cyclic_dataset ? 1 : 0);
    fprintf(f, "pubsub.enable_delta_frames=%d\n", pubrec->enable_delta_frames ? 1 : 0);
    fprintf(f, "pubsub.key_frame_count=%d\n",key_frame_count_int);
    fprintf(f, "pubsub.publishing_interval=%s\n", "!!!Specify publishing_interval");
    fprintf(f, "pubsub.DatasetMessageType=%s\n", delta == 1 ? dsmtype_to_string(pubrec->dsm_type[1]) : dsmtype_to_string(pubrec->dsm_type[0]));
    fprintf(f, "pubsub.publishedDataSetType=%s\n", published_dataset);
    fprintf(f, "pubsub.encoded_type=%s\n", encode_type);
    fprintf(f, "pubsub.connection_string=%s\n", connection_string);
    fprintf(f, "pubsub.keepalive_time=%d\n", keepalive_time_int);
    fprintf(f,
    ";For message types Control-Sync, Control-Async, and Control-Iso,\n"
    ";maxEncapsulatedDataSetMessageCount cannot be changed to\n"
    ";AscendingWriterID as it is not implemented yet, but undefined can be used. "
    ";For this purpose define the maximum number for maxEncapsulatedDataSetMessageCount.\n"
    ";For Event messages if undefined option is the choice, the same rule applies.\n"
    );
    fprintf(f, "pubsub.maxEncapsulatedDataSetMessageCount=%d\n", dataset_ordering);


    fprintf(f,
    ";---------------- Example DataSetField ----------------\n"
    ";This is a static example field to illustrate how a dataset field\n"
    ";can be configured. Replace or extend for real applications.\n"
    );
    fprintf(f, "dataset.field.count=1\n");
    /* Field 0 : Temperature */
    fprintf(f, "dataset.field.0.name=Temperature\n");
    fprintf(f, "dataset.field.0.description=Measured temperature value\n");
    fprintf(f, "dataset.field.0.datatype=Double\n");
    fprintf(f, "dataset.field.0.valueRank=-1\n");
    fprintf(f, "dataset.field.0.promotedField=0\n");             
    fprintf(f, "dataset.field.0.unit=degC\n");        
    fprintf(f, "dataset.field.0.initialValue=20.0\n");
    fprintf(f, "\n");

    fclose(f);
}


/* -------------------------
 * Output formatting
 * ------------------------- */

static void print_traffic_spec(const TrafficSpec *spec) {
    printf("\nTraffic characteristics (table row)\n");
    printf("----------------------------------\n");
    printf("Traffic ID             : %s\n", trafficid_to_string(spec->traffic_id));
    printf("Application Class      : %s\n", spec->application_class);
    printf("Traffic Type           : %s\n", spec->traffic_type);
    printf("Cycle Time (T)         : %s\n", spec->cycle_time);
    printf("Transmission Guarantee : %s\n", spec->transmission_guarantee);
    printf("Latency (L)            : %s\n", spec->latency);
    printf("Criticality            : %s\n", spec->criticality);
    printf("Jitter Tolerance       : %s\n", spec->jitter_tolerance);
    printf("Loss Tolerance         : %s\n", spec->loss_tolerance);
    printf("Length Variability     : %s\n", spec->length_variability);
    printf("Length (Byte)          : %s\n", spec->length);
}

static void print_tsn_decision(const TSNDecision *dec) {
    printf("\nTSN class decision\n");
    printf("------------------\n");
    if (dec->has_alternative) {
        printf("Primary TSN class         : %s\n", tsnclass_to_string(dec->primary));
        printf("Alternative (if justified): %s\n", tsnclass_to_string(dec->alternative));
    } else {
        printf("TSN class                 : %s\n", tsnclass_to_string(dec->primary));
    }
    printf("Derived flags (P,D,JO,HRT): (%d,%d,%d,%d)\n", dec->P, dec->D, dec->JO, dec->HRT);
}

static void print_pubsub_recommendation(const PubSubRecommendation *rec) {
    printf("\nOPC UA PubSub mapping (recommended)\n");
    printf("----------------------------------\n");
    printf("CyclicDataset                              : %s\n", rec->cyclic_dataset ? "True" : "False");

    printf("DSM Type                                   : ");
    for (size_t i = 0; i < rec->dsm_type_count; i++) {
        printf("%s", dsmtype_to_string(rec->dsm_type[i]));
        if (i + 1 < rec->dsm_type_count) printf(",");
    }
    printf("\n");

    printf("EnableDeltaFrames (DSM Type = Delta Frame) : %s\n", rec->enable_delta_frames ? "True" : "False");
    printf("KeyFrameCount                              : %s\n", rec->key_frame_count_text ? rec->key_frame_count_text : "N/A");
    printf("PublishingInterval                         : %s\n", rec->publishing_interval ? rec->publishing_interval : "N/A");

    printf("KeepAliveTime (hint)                       : %s\n",
           (rec->keep_alive_time_hint && rec->keep_alive_time_hint[0] != '\0')
               ? rec->keep_alive_time_hint
               : "Not recommended / not required");

    printf("DatasetOrdering                            : ");
    for (size_t i = 0; i < rec->dataset_ordering_count; i++) {
        printf("%s", datasetordering_to_string(rec->dataset_ordering[i]));
        if (i + 1 < rec->dataset_ordering_count) printf(",");
    }
    printf("\n");

    printf("EncodingMimeType                           : ");
    for (size_t i = 0; i < rec->encoding_mime_type_count; i++) {
        printf("%s", encoding_to_string(rec->encoding_mime_type[i]));
        if (i + 1 < rec->encoding_mime_type_count) printf(",");
    }
    printf("\n");

    printf("TransportProfileUri                        : ");
    for (size_t i = 0; i < rec->transport_profile_uri_count; i++) {
        printf("%s", transport_to_string(rec->transport_profile_uri[i]));
        if (i + 1 < rec->transport_profile_uri_count) printf(",");
    }
    printf("\n");
}

/* -------------------------
 * Main
 * ------------------------- */

int main(void) {
    printf("OPC UA TSN Converged configuration recommendations\n");

    // Config cfg = {0};

    // if (load_config_ini("input.ini", &cfg) != 0)
    // {
    //     printf("Failed to load config\n");
    //     return 1;
    // }

    Config flows[MAX_FLOWS];
    int flow_count = load_bulk_config_ini("input.ini", flows, MAX_FLOWS);

    if (flow_count <= 0) {
        printf("Failed to load config or no flows found.\n");
        return 1;
    }

    for (int i = 0; i < flow_count; i++) {
        Config *cfg = &flows[i];

        TrafficID traffic_id = infer_traffic_id(cfg->periodic, cfg->traffic_application_class);
        if (traffic_id == TRAFFIC_DEFAULT) {
            printf("\n[%s] Invalid traffic_application_class=%d\n",
                   cfg->flow_id, cfg->traffic_application_class);
            continue;
        }

        const TrafficSpec *spec = &TABLE[traffic_id];
        TSNDecision tsn = classify_tsn(cfg->periodic,
                                       spec->has_deadline_or_bounded_latency,
                                       spec->low_jitter,
                                       spec->high_critical);

        printf("\nResult\n");
        printf("======\n");
        print_traffic_spec(spec);
        print_tsn_decision(&tsn);

        if (traffic_id != TRAFFIC_NETWORK) {
            PubSubRecommendation pubrec = recommend_pubsub_config(traffic_id, spec);
            print_pubsub_recommendation(&pubrec);

            char outname[128];
            snprintf(outname, sizeof(outname), "reco_%s.ini", cfg->flow_id);
            write_reco_ini(outname, traffic_id, &tsn, &pubrec);

            printf("Generated %s\n", outname);
        } else {
             printf("For Traffic ID = Network, no pubsub configuration is generated as the source of these traffic types are not publishers or subscribers.\n");
        }
    }   
    return 0;
}