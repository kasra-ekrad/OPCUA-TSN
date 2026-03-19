/*
 * gen_publisher.c
 *
 * Reads key=value reco.ini and generates a ready-made publisher code
 * (open62541 UDP/UADP publisher example).
 *
 * Compile:
 *   gcc -std=c11 -Wall -Wextra -O2 CodeGenerator.c -o CodeGenerator
 *
 * Run:
 *   ./CodeGenerator reco.ini publisher_generated.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 512
#define MAX_VAL  512

typedef struct {
    char connection_name[MAX_VAL];            /* pubsub.connection_name */
    unsigned publisher_id;                    /* pubsub.publisher_id */
    char pds_name[MAX_VAL];                   /* pubsub.publishedDataSetConfig_name */
    unsigned writer_group_id;                 /* pubsub.writerGroupId */
    char writer_group_name[MAX_VAL];          /* pubsub.writerGroupConfig_name */
    unsigned data_set_writer_id;              /* pubsub.dataSetWriterId */
    char data_set_writer_name[MAX_VAL];       /* pubsub.dataSetWriterName */
    char transport_profile[MAX_VAL];          /* pubsub.transportProfile (URI) */
    char encoding_mime_type[MAX_VAL];         /* pubsub.encodingMimeType */
    int  cyclic_dataset;                      /* pubsub.cyclic_dataset */
    int  enable_delta_frames;                 /* pubsub.enable_delta_frames */
    int  key_frame_count;                     /* pubsub.key_frame_count */
    unsigned publishing_interval_us;          /* pubsub.publishing_interval (microseconds) */
    char dataset_message_type[MAX_VAL];       /* pubsub.DatasetMessageType: KeyFrame/DeltaFrame/Event */
    char published_dataset_type[MAX_VAL];     /* pubsub.publishedDataSetType */
    char encoded_type[MAX_VAL];               /* pubsub.encoded_type */
    char connection_string[MAX_VAL];          /* pubsub.connection_string */
    unsigned keepalive_time;                  /* pubsub.keepalive_time */
    int max_encapsulated_dsm_count;           /* pubsub.maxEncapsulatedDataSetMessageCount (may be -1) */
    char interface_name[MAX_VAL];             /* pubsub.interface */
    int dataset_field_count;                  /* dataset.field.count */
    char field0_name[MAX_VAL];                /* dataset.field.0.name */
    char field0_description[MAX_VAL];         /* dataset.field.0.description */
    char field0_datatype[MAX_VAL];            /* dataset.field.0.datatype (e.g., Double) */
    int  field0_valueRank;                    /* dataset.field.0.valueRank (-1 scalar) */
    int  field0_promotedField;                /* dataset.field.0.promotedField (0/1) */
    char field0_unit[MAX_VAL];                /* dataset.field.0.unit */
    double field0_initialValue;               /* dataset.field.0.initialValue */

} Reco;

static void trim(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static void reco_init_defaults(Reco *r) {
    memset(r, 0, sizeof(*r));

    strcpy(r->connection_name, "Connection_Name_Demo");
    r->publisher_id = 2525;
    strcpy(r->pds_name, "PDS_Demo");
    r->writer_group_id = 100;
    strcpy(r->writer_group_name, "WriterGroup_Demo");
    r->data_set_writer_id = 200;
    strcpy(r->data_set_writer_name, "dataSetWriter_Demo");

    strcpy(r->transport_profile, "UA_PUBSUB_TRANSPORTPROFILE_UDP_UADP");
    strcpy(r->encoding_mime_type, "UA_PUBSUB_ENCODING_UADP");
    r->cyclic_dataset = 1;
    r->enable_delta_frames = 0;
    r->key_frame_count = 1;
    r->publishing_interval_us = 1000; /* 1ms */
    strcpy(r->dataset_message_type, "KeyFrame");
    strcpy(r->published_dataset_type, "UA_PUBSUB_DATASET_PUBLISHEDITEMS");
    strcpy(r->encoded_type, "UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE");
    strcpy(r->connection_string, "opc.udp://127.0.0.1:4840/");
    r->keepalive_time = 0;
    r->max_encapsulated_dsm_count = -1;

    strcpy(r->interface_name, "");

    r->dataset_field_count = 0;
    strcpy(r->field0_name, "Temperature");
    strcpy(r->field0_description, "Measured temperature value");
    strcpy(r->field0_datatype, "Double");
    r->field0_valueRank = -1;
    r->field0_promotedField = 0;
    strcpy(r->field0_unit, "degC");
    r->field0_initialValue = 20.0;
}

static void set_kv(Reco *r, const char *key, const char *val) {
    if (strcmp(key, "pubsub.connection_name") == 0) {
        strncpy(r->connection_name, val, sizeof(r->connection_name) - 1);
    } else if (strcmp(key, "pubsub.publisher_id") == 0) {
        r->publisher_id = (unsigned)strtoul(val, NULL, 10);
    } else if (strcmp(key, "pubsub.publishedDataSetConfig_name") == 0) {
        strncpy(r->pds_name, val, sizeof(r->pds_name) - 1);
    } else if (strcmp(key, "pubsub.writerGroupId") == 0) {
        r->writer_group_id = (unsigned)strtoul(val, NULL, 10);
    } else if (strcmp(key, "pubsub.writerGroupConfig_name") == 0) {
        strncpy(r->writer_group_name, val, sizeof(r->writer_group_name) - 1);
    } else if (strcmp(key, "pubsub.dataSetWriterId") == 0) {
        r->data_set_writer_id = (unsigned)strtoul(val, NULL, 10);
    } else if (strcmp(key, "pubsub.dataSetWriterName") == 0) {
        strncpy(r->data_set_writer_name, val, sizeof(r->data_set_writer_name) - 1);

    } else if (strcmp(key, "pubsub.transportProfile") == 0) {
        strncpy(r->transport_profile, val, sizeof(r->transport_profile) - 1);
    } else if (strcmp(key, "pubsub.encodingMimeType") == 0) {
        strncpy(r->encoding_mime_type, val, sizeof(r->encoding_mime_type) - 1);
    } else if (strcmp(key, "pubsub.cyclic_dataset") == 0) {
        r->cyclic_dataset = atoi(val) != 0;
    } else if (strcmp(key, "pubsub.enable_delta_frames") == 0) {
        r->enable_delta_frames = atoi(val) != 0;
    } else if (strcmp(key, "pubsub.key_frame_count") == 0) {
        r->key_frame_count = atoi(val);
        if (r->key_frame_count < 1) r->key_frame_count = 1;
    } else if (strcmp(key, "pubsub.publishing_interval") == 0 ||
               strcmp(key, "pubsub.publishing_interval_us") == 0) {
        unsigned long v = strtoul(val, NULL, 10);
        if (v == 0) v = 1000;
        r->publishing_interval_us = (unsigned)v;
    } else if (strcmp(key, "pubsub.DatasetMessageType") == 0) {
        strncpy(r->dataset_message_type, val, sizeof(r->dataset_message_type) - 1);
    } else if (strcmp(key, "pubsub.publishedDataSetType") == 0) {
        strncpy(r->published_dataset_type, val, sizeof(r->published_dataset_type) - 1);
    } else if (strcmp(key, "pubsub.encoded_type") == 0) {
        strncpy(r->encoded_type, val, sizeof(r->encoded_type) - 1);
    } else if (strcmp(key, "pubsub.connection_string") == 0) {
        strncpy(r->connection_string, val, sizeof(r->connection_string) - 1);
    } else if (strcmp(key, "pubsub.keepalive_time") == 0) {
        r->keepalive_time = (unsigned)strtoul(val, NULL, 10);
    } else if (strcmp(key, "pubsub.maxEncapsulatedDataSetMessageCount") == 0) {
        r->max_encapsulated_dsm_count = atoi(val);
    } else if (strcmp(key, "pubsub.interface") == 0) {
        strncpy(r->interface_name, val, sizeof(r->interface_name) - 1);

    } else if (strcmp(key, "dataset.field.count") == 0) {
        r->dataset_field_count = atoi(val);
    } else if (strcmp(key, "dataset.field.0.name") == 0) {
        strncpy(r->field0_name, val, sizeof(r->field0_name) - 1);
    } else if (strcmp(key, "dataset.field.0.description") == 0) {
        strncpy(r->field0_description, val, sizeof(r->field0_description) - 1);
    } else if (strcmp(key, "dataset.field.0.datatype") == 0) {
        strncpy(r->field0_datatype, val, sizeof(r->field0_datatype) - 1);
    } else if (strcmp(key, "dataset.field.0.valueRank") == 0) {
        r->field0_valueRank = atoi(val);
    } else if (strcmp(key, "dataset.field.0.promotedField") == 0) {
        r->field0_promotedField = atoi(val) != 0;
    } else if (strcmp(key, "dataset.field.0.unit") == 0) {
        strncpy(r->field0_unit, val, sizeof(r->field0_unit) - 1);
    } else if (strcmp(key, "dataset.field.0.initialValue") == 0) {
        r->field0_initialValue = strtod(val, NULL);
    }
}

static int parse_ini(const char *path, Reco *r) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return 0;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char *hash = strchr(line, '#');
        if (hash) *hash = '\0';
        char *semi = strchr(line, ';');
        if (semi) *semi = '\0';

        trim(line);
        if (line[0] == '\0') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        trim(key);
        trim(val);
        if (key[0] == '\0') continue;

        set_kv(r, key, val);
    }

    fclose(f);
    return 1;
}

static int supported(const Reco *r) {
    if (strstr(r->transport_profile, "pubsub-udp-uadp") == NULL)
        return 0;
    if (strcmp(r->encoding_mime_type, "UA_PUBSUB_ENCODING_UADP") != 0)
        return 0;
    return 1;
}

static void emit_publisher_c(FILE *out, const Reco *r) {
    double pub_ms = (double)r->publishing_interval_us / 1000.0;
    if (pub_ms <= 0.0) pub_ms = 1.0;

    fprintf(out,
        "/*\n"
        " * publisher_generated.c\n"
        " * Auto-generated from reco.ini\n"
        " */\n\n"
    );

    fprintf(out,
        "#include <open62541/server.h>\n"
        "#include <open62541/server_config_default.h>\n"
        "#include <open62541/plugin/log_stdout.h>\n"
        "#include <open62541/pubsub.h>\n\n"
    );

    fprintf(out,
        "static UA_NodeId addTemperatureVariable(UA_Server *server) {\n"
        "    UA_Double initial = %.6f;\n"
        "    UA_VariableAttributes attr = UA_VariableAttributes_default;\n"
        "    UA_Variant_setScalar(&attr.value, &initial, &UA_TYPES[UA_TYPES_DOUBLE]);\n"
        "    attr.displayName = UA_LOCALIZEDTEXT(\"en-US\", \"%s\");\n"
        "    attr.description = UA_LOCALIZEDTEXT(\"en-US\", \"%s\");\n"
        "    attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;\n"
        "    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;\n"
        "    UA_NodeId varNodeId = UA_NODEID_STRING(1, \"Demo.%s\");\n"
        "    UA_QualifiedName qn = UA_QUALIFIEDNAME(1, \"%s\");\n"
        "    UA_NodeId parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);\n"
        "    UA_NodeId parentRef = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);\n"
        "    UA_NodeId varType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);\n"
        "    UA_Server_addVariableNode(server, varNodeId, parent, parentRef, qn, varType, attr, NULL, NULL);\n"
        "    return varNodeId;\n"
        "}\n\n",
        r->field0_initialValue,
        r->field0_name,
        r->field0_description,
        r->field0_name,
        r->field0_name
    );

    fprintf(out,
        "static void addPublishedDataSet(UA_Server *server, UA_NodeId tempVar,\n"
        "                                UA_NodeId *pdsId, UA_NodeId *fieldId) {\n"
        "    UA_PublishedDataSetConfig pdsConfig;\n"
        "    memset(&pdsConfig, 0, sizeof(pdsConfig));\n"
        "    pdsConfig.name = UA_STRING(\"%s\");\n"
        "    pdsConfig.publishedDataSetType = %s;\n"
        "    UA_Server_addPublishedDataSet(server, &pdsConfig, pdsId);\n\n"
        "    UA_DataSetFieldConfig fieldConfig;\n"
        "    memset(&fieldConfig, 0, sizeof(fieldConfig));\n"
        "    fieldConfig.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;\n"
        "    fieldConfig.field.variable.publishParameters.publishedVariable = tempVar;\n"
        "    fieldConfig.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;\n"
        "    fieldConfig.field.variable.promotedField = %s;\n"
        "    fieldConfig.dataSetFieldContentMask = UA_DATASETFIELDCONTENTMASK_RAWDATA;\n"
        "    UA_Server_addDataSetField(server, *pdsId, &fieldConfig, fieldId);\n"
        "}\n\n",
        r->pds_name,
        r->published_dataset_type,
        r->field0_promotedField ? "UA_TRUE" : "UA_FALSE"
        
    );

    fprintf(out,
        "static void addPubSubConnection(UA_Server *server, UA_NodeId *connectionId) {\n"
        "    UA_PubSubConnectionConfig connectionConfig;\n"
        "    memset(&connectionConfig, 0, sizeof(connectionConfig));\n"
        "    connectionConfig.name = UA_STRING(\"%s\");\n"
        "    connectionConfig.transportProfileUri = UA_STRING(\"%s\");\n"
        "    connectionConfig.enabled = UA_TRUE;\n"
        "    connectionConfig.publisherId.numeric = %u;\n\n"
        "    UA_NetworkAddressUrlDataType addr = UA_NETWORKADDRESSURLDATATYPE_NEW();\n"
        "    addr.networkInterface = UA_STRING(\"%s\");\n"
        "    addr.url = UA_STRING(\"%s\");\n"
        "    UA_Variant_setScalar(&connectionConfig.address, &addr,\n"
        "                         &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);\n"
        "    UA_Server_addPubSubConnection(server, &connectionConfig, connectionId);\n"
        "    UA_NetworkAddressUrlDataType_clear(&addr);\n"
        "}\n\n",
        r->connection_name,
        r->transport_profile,
        r->publisher_id,
        (r->interface_name[0] ? r->interface_name : ""),
        r->connection_string
    );

    fprintf(out,
        "static void addWriterGroupAndDataSetWriter(UA_Server *server,\n"
        "                                          UA_NodeId connectionId,\n"
        "                                          UA_NodeId pdsId,\n"
        "                                          UA_NodeId *writerGroupId,\n"
        "                                          UA_NodeId *dataSetWriterId) {\n"
        "    UA_WriterGroupConfig writerGroupConfig;\n"
        "    memset(&writerGroupConfig, 0, sizeof(writerGroupConfig));\n"
        "    writerGroupConfig.name = UA_STRING(\"%s\");\n"
        "    writerGroupConfig.publishingInterval = %.6f; /* ms */\n"
        "    writerGroupConfig.enabled = UA_TRUE;\n"
        "    writerGroupConfig.writerGroupId = %u;\n"
        "    writerGroupConfig.encodingMimeType = %s;\n\n"
        "    UA_UadpWriterGroupMessageDataType writerGroupMessage;\n"
        "    memset(&writerGroupMessage, 0, sizeof(writerGroupMessage));\n"
        "    writerGroupMessage.networkMessageContentMask = (UA_UadpNetworkMessageContentMask)\n"
        "        (UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |\n"
        "         UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |\n"
        "         UA_UADPNETWORKMESSAGECONTENTMASK_GROUPVERSION |\n"
        "         UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |\n"
        "         UA_UADPNETWORKMESSAGECONTENTMASK_SEQUENCENUMBER |\n"
        "         UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER);\n"
        "    writerGroupConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;\n"
        "    writerGroupConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE];\n"
        "    writerGroupConfig.messageSettings.content.decoded.data = &writerGroupMessage;\n\n"
        "    UA_Server_addWriterGroup(server, connectionId, &writerGroupConfig, writerGroupId);\n\n"
        "    UA_DataSetWriterConfig dataSetWriterConfig;\n"
        "    memset(&dataSetWriterConfig, 0, sizeof(dataSetWriterConfig));\n"
        "    dataSetWriterConfig.name = UA_STRING(\"%s\");\n"
        "    dataSetWriterConfig.dataSetWriterId = %u;\n"
        "    dataSetWriterConfig.keyFrameCount = %u;\n\n"
        "    UA_UadpDataSetWriterMessageDataType uadpDataSetWriterMessageDataType;\n"
        "    memset(&uadpDataSetWriterMessageDataType, 0, sizeof(uadpDataSetWriterMessageDataType));\n"
        "    uadpDataSetWriterMessageDataType.dataSetMessageContentMask = UA_UADPDATASETMESSAGECONTENTMASK_SEQUENCENUMBER;\n"
        "    dataSetWriterConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;\n"
        "    dataSetWriterConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPDATASETWRITERMESSAGEDATATYPE];\n"
        "    dataSetWriterConfig.messageSettings.content.decoded.data = &uadpDataSetWriterMessageDataType;\n\n"
        "    UA_Server_addDataSetWriter(server, *writerGroupId, pdsId, &dataSetWriterConfig, dataSetWriterId);\n"
        "}\n\n",
        r->writer_group_name,
        pub_ms,
        r->writer_group_id,
        r->encoding_mime_type,
        r->data_set_writer_name,
        r->data_set_writer_id,
        (unsigned)r->key_frame_count
    );

    fprintf(out,
        "int main(void) {\n"
        "    UA_Server *server = UA_Server_new();\n"
        "    UA_ServerConfig *config = UA_Server_getConfig(server);\n"
        "    UA_ServerConfig_setDefault(config);\n\n"
        "    UA_NodeId tempVar = addTemperatureVariable(server);\n\n"
        "    UA_NodeId pdsId, fieldId;\n"
        "    addPublishedDataSet(server, tempVar, &pdsId, &fieldId);\n\n"
        "    UA_NodeId connectionId;\n"
        "    addPubSubConnection(server, &connectionId);\n\n"
        "    UA_NodeId writerGroupId, dataSetWriterId;\n"
        "    addWriterGroupAndDataSetWriter(server, connectionId, pdsId, &writerGroupId, &dataSetWriterId);\n\n"
        "    UA_StatusCode retval = UA_Server_run(server, &(UA_Boolean){true});\n"
        "    UA_Server_delete(server);\n"
        "    return (retval == UA_STATUSCODE_GOOD) ? EXIT_SUCCESS : EXIT_FAILURE;\n"
        "}\n"
    );
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <reco.ini> <publisher_generated.c>\\n", argv[0]);
        return 2;
    }

    Reco r;
    reco_init_defaults(&r);

    if (!parse_ini(argv[1], &r)) {
        fprintf(stderr, "Failed to parse %s\\n", argv[1]);
        return 1;
    }

    if (!supported(&r)) {
        fprintf(stderr,
                "Only UDP/UADP + UA_PUBSUB_ENCODING_UADP is supported by this generator version.\\n"
                "transportProfile=%s\\nencodingMimeType=%s\\n",
                r.transport_profile, r.encoding_mime_type);
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        perror("fopen");
        return 1;
    }

    emit_publisher_c(out, &r);
    fclose(out);

    printf("Generated: %s\\n", argv[2]);
    return 0;
}
