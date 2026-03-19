/*
 * publisher_generated.c
 * Auto-generated from reco.ini
 */

#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/pubsub.h>

static UA_NodeId addTemperatureVariable(UA_Server *server) {
    UA_Double initial = 20.000000;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Variant_setScalar(&attr.value, &initial, &UA_TYPES[UA_TYPES_DOUBLE]);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Temperature");
    attr.description = UA_LOCALIZEDTEXT("en-US", "Measured temperature value");
    attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_NodeId varNodeId = UA_NODEID_STRING(1, "Demo.Temperature");
    UA_QualifiedName qn = UA_QUALIFIEDNAME(1, "Temperature");
    UA_NodeId parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentRef = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId varType = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, varNodeId, parent, parentRef, qn, varType, attr, NULL, NULL);
    return varNodeId;
}

static void addPublishedDataSet(UA_Server *server, UA_NodeId tempVar,
                                UA_NodeId *pdsId, UA_NodeId *fieldId) {
    UA_PublishedDataSetConfig pdsConfig;
    memset(&pdsConfig, 0, sizeof(pdsConfig));
    pdsConfig.name = UA_STRING("PDS_Demo");
    pdsConfig.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
    UA_Server_addPublishedDataSet(server, &pdsConfig, pdsId);

    UA_DataSetFieldConfig fieldConfig;
    memset(&fieldConfig, 0, sizeof(fieldConfig));
    fieldConfig.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;
    fieldConfig.field.variable.publishParameters.publishedVariable = tempVar;
    fieldConfig.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;
    fieldConfig.field.variable.promotedField = UA_FALSE;
    /* Requested */
    fieldConfig.dataSetFieldContentMask = UA_DATASETFIELDCONTENTMASK_RAWDATA;
    UA_Server_addDataSetField(server, *pdsId, &fieldConfig, fieldId);
}

static void addPubSubConnection(UA_Server *server, UA_NodeId *connectionId) {
    UA_PubSubConnectionConfig connectionConfig;
    memset(&connectionConfig, 0, sizeof(connectionConfig));
    connectionConfig.name = UA_STRING("Connection_Name_Demo");
    connectionConfig.transportProfileUri = UA_STRING(UA_PUBSUB_TRANSPORTPROFILE_UDP_UADP);
    connectionConfig.enabled = UA_TRUE;
    connectionConfig.publisherId.numeric = 2525;

    UA_NetworkAddressUrlDataType addr = UA_NETWORKADDRESSURLDATATYPE_NEW();
    addr.networkInterface = UA_STRING("");
    addr.url = UA_STRING("opc.udp:!!!Specify network address e.g. \\127.0.0.1:1883");
    UA_Variant_setScalar(&connectionConfig.address, &addr,
                         &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
    UA_Server_addPubSubConnection(server, &connectionConfig, connectionId);
    UA_NetworkAddressUrlDataType_clear(&addr);
}

static void addWriterGroupAndDataSetWriter(UA_Server *server,
                                          UA_NodeId connectionId,
                                          UA_NodeId pdsId,
                                          UA_NodeId *writerGroupId,
                                          UA_NodeId *dataSetWriterId) {
    UA_WriterGroupConfig writerGroupConfig;
    memset(&writerGroupConfig, 0, sizeof(writerGroupConfig));
    writerGroupConfig.name = UA_STRING("WriterGroup_Demo");
    writerGroupConfig.publishingInterval = 1.000000; /* ms */
    writerGroupConfig.enabled = UA_TRUE;
    writerGroupConfig.writerGroupId = 100;
    writerGroupConfig.encodingMimeType = UA_PUBSUB_ENCODING_UADP;

    /* Requested networkMessageContentMask */
    UA_UadpWriterGroupMessageDataType writerGroupMessage;
    memset(&writerGroupMessage, 0, sizeof(writerGroupMessage));
    writerGroupMessage.networkMessageContentMask = (UA_UadpNetworkMessageContentMask)
        (UA_UADPNETWORKMESSAGECONTENTMASK_PUBLISHERID |
         UA_UADPNETWORKMESSAGECONTENTMASK_GROUPHEADER |
         UA_UADPNETWORKMESSAGECONTENTMASK_GROUPVERSION |
         UA_UADPNETWORKMESSAGECONTENTMASK_WRITERGROUPID |
         UA_UADPNETWORKMESSAGECONTENTMASK_SEQUENCENUMBER |
         UA_UADPNETWORKMESSAGECONTENTMASK_PAYLOADHEADER);
    writerGroupConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    writerGroupConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPWRITERGROUPMESSAGEDATATYPE];
    writerGroupConfig.messageSettings.content.decoded.data = &writerGroupMessage;

    UA_Server_addWriterGroup(server, connectionId, &writerGroupConfig, writerGroupId);

    UA_DataSetWriterConfig dataSetWriterConfig;
    memset(&dataSetWriterConfig, 0, sizeof(dataSetWriterConfig));
    dataSetWriterConfig.name = UA_STRING("dataSetWriter_Demo");
    dataSetWriterConfig.dataSetWriterId = 200;
    dataSetWriterConfig.keyFrameCount = 1;

    /* Requested: DataSetMessage content mask */
    UA_UadpDataSetWriterMessageDataType uadpDataSetWriterMessageDataType;
    memset(&uadpDataSetWriterMessageDataType, 0, sizeof(uadpDataSetWriterMessageDataType));
    uadpDataSetWriterMessageDataType.dataSetMessageContentMask = UA_UADPDATASETMESSAGECONTENTMASK_SEQUENCENUMBER;
    dataSetWriterConfig.messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    dataSetWriterConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_UADPDATASETWRITERMESSAGEDATATYPE];
    dataSetWriterConfig.messageSettings.content.decoded.data = &uadpDataSetWriterMessageDataType;

    UA_Server_addDataSetWriter(server, *writerGroupId, pdsId, &dataSetWriterConfig, dataSetWriterId);
}

int main(void) {
    UA_Server *server = UA_Server_new();
    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_ServerConfig_setDefault(config);

    /* Demo variable node (Temperature) */
    UA_NodeId tempVar = addTemperatureVariable(server);

    /* PubSub setup */
    UA_NodeId pdsId, fieldId;
    addPublishedDataSet(server, tempVar, &pdsId, &fieldId);

    UA_NodeId connectionId;
    addPubSubConnection(server, &connectionId);

    UA_NodeId writerGroupId, dataSetWriterId;
    addWriterGroupAndDataSetWriter(server, connectionId, pdsId, &writerGroupId, &dataSetWriterId);

    UA_StatusCode retval = UA_Server_run(server, &(UA_Boolean){true});
    UA_Server_delete(server);
    return (retval == UA_STATUSCODE_GOOD) ? EXIT_SUCCESS : EXIT_FAILURE;
}
