/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>

static UA_Boolean running = true;
static void stopHandler(int sig) {
    running = false;
}

int main(void) {
    signal(SIGINT,  stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    /* Create a rudimentary objectType
     *
     * BaseObjectType
     * |
     * +- (OT) MamalType
     *    + (V) Age
     *    + (OT) DogType
     *      |
     *      + (V) Name
     */
    UA_StatusCode retval;
    UA_ObjectTypeAttributes otAttr = UA_ObjectTypeAttributes_default;
    otAttr.description = UA_LOCALIZEDTEXT("en-US", "A mamal");
    otAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MamalType");
    UA_Server_addObjectTypeNode(server, UA_NODEID_NUMERIC(1, 10000),
                                UA_NS0ID(BASEOBJECTTYPE), UA_NS0ID(HASSUBTYPE),
                                UA_QUALIFIEDNAME(1, "MamalType"), otAttr, NULL, NULL);

    UA_VariableAttributes vAttr = UA_VariableAttributes_default;
    vAttr.description =  UA_LOCALIZEDTEXT("en-US", "This mamals Age in months");
    vAttr.displayName =  UA_LOCALIZEDTEXT("en-US", "Age");
    UA_UInt32 ageVar = 0;
    UA_Variant_setScalar(&vAttr.value, &ageVar, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 10001),
                              UA_NODEID_NUMERIC(1, 10000), UA_NS0ID(HASPROPERTY),
                              UA_QUALIFIEDNAME(1, "Age"),
                              UA_NS0ID(BASEDATAVARIABLETYPE), vAttr, NULL, NULL);

    UA_Server_addReference(server, UA_NODEID_NUMERIC(1, 10001),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),
                              UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_MODELLINGRULE_MANDATORY), true);

    otAttr = UA_ObjectTypeAttributes_default;
    otAttr.description = UA_LOCALIZEDTEXT("en-US", "A dog, subtype of mamal");
    otAttr.displayName = UA_LOCALIZEDTEXT("en-US", "DogType");
    UA_Server_addObjectTypeNode(server, UA_NODEID_NUMERIC(1, 10002),
                                UA_NODEID_NUMERIC(1, 10000), UA_NS0ID(HASSUBTYPE),
                                UA_QUALIFIEDNAME(1, "DogType"), otAttr, NULL, NULL);

    vAttr = UA_VariableAttributes_default;
    vAttr.description =  UA_LOCALIZEDTEXT("en-US", "This dogs Name");
    vAttr.displayName =  UA_LOCALIZEDTEXT("en-US", "Name");
    UA_String defaultName = UA_STRING("unnamed dog");
    UA_Variant_setScalar(&vAttr.value, &defaultName, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 10003),
                              UA_NODEID_NUMERIC(1, 10002), UA_NS0ID(HASPROPERTY),
                              UA_QUALIFIEDNAME(1, "Name"), UA_NS0ID(BASEDATAVARIABLETYPE),
                              vAttr, NULL, NULL);

    /* Instatiate a dog named bello:
     * (O) Objects
     *   + O Bello <DogType>
     *     + Age
     */

    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en-US", "A dog named Bello with only mandatory fields");
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Bello");
    UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, 0),
                            UA_NS0ID(OBJECTSFOLDER), UA_NS0ID(HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Bello"), UA_NODEID_NUMERIC(1, 10002),
                            oAttr, NULL, NULL);

    /* Instatiate a dog named doge:
     * (O) Objects
     *   + O Bello <DogType>
     *     + Age
     *     + Name (optional)
     */
    oAttr = UA_ObjectAttributes_default;
    oAttr.description = UA_LOCALIZEDTEXT("en-US", "A dog named Doge with optional fields");
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Doge");
    UA_NodeId dogeNodeId;
    UA_Server_addNode_begin(server, UA_NODECLASS_OBJECT, UA_NODEID_NUMERIC(1, 0),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Doge"), UA_NODEID_NUMERIC(1, 10002),
                            &oAttr, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES],
                            NULL, &dogeNodeId);
    
    vAttr = UA_VariableAttributes_default;
    vAttr.description =  UA_LOCALIZEDTEXT("en-US", "This dogs Name");
    vAttr.displayName =  UA_LOCALIZEDTEXT("en-US", "Name");
    UA_String dogeName = UA_STRING("Doge");
    UA_Variant_setScalar(&vAttr.value, &dogeName, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, 0),
                                dogeNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                UA_QUALIFIEDNAME(1, "Name"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), vAttr, NULL, NULL);

    UA_Server_addNode_finish(server, dogeNodeId);

    retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
