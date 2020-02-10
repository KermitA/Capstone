//Prep a packet
cJSON * prepSetPacket()
{
	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "NetID", networkID);
	cJSON_AddItemToObject(root, "DestID", destID);
	cJSON_AddItemToObject(root, "SourceID", srcID);
	cJSON_AddItemToObject(root, "Version", protocolVersion);
	cJSON_AddItemToObject(root, "Length", length);
	cJSON_AddItemToObject(root, "Direction", 1);
	cJSON_AddItemToObject(root, "Graphic", 0);
	cJSON_AddItemToObject(root, "Color", color);
	cJSON_AddItemToObject(root, "Duration", duration);
	
	return root;
}


//Parse a Packet
void parsePacket()
{
	cJSON *root = cJSON_Parse(inputStr);
	int networkID = cJSON_GetObjectItem(root, "NetID");
	int destID = cJSON_GetObjectItem(root, "DestID");
	int srcID = cJSON_GetObjectItem(root, "SourceID");
	int protocolVersion = cJSON_GetObjectItem(root, "Version");
	int length = cJSON_GetObjectItem(root, "Length");
	int msgDirection = cJSON_GetObjectItem(root, "Direction");
	int graphic = cJSON_GetObjectItem(root, "Graphic");
	int color = cJSON_GetObjectItem(root, "Color");
	int duration = cJSON_GetObjectItem(root, "Duration");
	
	cJSON_Delete(root);
}
