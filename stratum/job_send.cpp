#include "stratum.h"

static int g_job_next_id = rand()/1000;

int job_get_jobid()
{
	CommonLock(&g_job_create_mutex);
	int jobid = ++g_job_next_id + rand()/1000;

	CommonUnlock(&g_job_create_mutex);
	return jobid;
}

static void job_mining_notify_buffer(YAAMP_JOB *job, char *buffer)
{
	YAAMP_JOB_TEMPLATE *templ = job->templ;

	if (!strcmp(g_stratum_algo, "lbry")) {
		sprintf(buffer, "{\"id\":null,\"method\":\"mining.notify\",\"params\":["
			"\"%x\",\"%s\",\"%s\",\"%s\",\"%s\",[%s],\"%s\",\"%s\",\"%s\",true]}\n",
			job->id, templ->prevhash_be, templ->claim_be, templ->coinb1, templ->coinb2,
			templ->txmerkles, templ->version, templ->nbits, templ->ntime);
		return;
	} else if (strlen(templ->extradata_hex) == 128) {
		// LUX smart contract state hashes (like lbry extra field, here the 2 root hashes in one)
		sprintf(buffer, "{\"id\":null,\"method\":\"mining.notify\",\"params\":["
			"\"%x\",\"%s\",\"%s\",\"%s\",\"%s\",[%s],\"%s\",\"%s\",\"%s\",true]}\n",
			job->id, templ->prevhash_be, templ->extradata_be, templ->coinb1, templ->coinb2,
			templ->txmerkles, templ->version, templ->nbits, templ->ntime);
		return;
	}

	// standard stratum
	sprintf(buffer, "{\"id\":null,\"method\":\"mining.notify\",\"params\":[\"%x\",\"%s\",\"%s\",\"%s\",[%s],\"%s\",\"%s\",\"%s\",true]}\n",
		job->id, templ->prevhash_be, templ->coinb1, templ->coinb2, templ->txmerkles, templ->version, templ->nbits, templ->ntime);
}

static int job_mining_notify_buffer_mtp_send(YAAMP_CLIENT *client,YAAMP_JOB *job, YAAMP_SOCKET *s)
{
	YAAMP_JOB_TEMPLATE *templ = job->templ;

	///binlify the whole stuff
	unsigned char *prevhash_be = (unsigned char*)malloc(strlen(templ->prevhash_be)/2);
	binlify(prevhash_be, templ->prevhash_be);
	unsigned char *coinb1 = (unsigned char*)malloc(strlen(templ->coinb1) / 2);
	binlify(coinb1, templ->coinb1);
	unsigned char *coinb2 = (unsigned char*)malloc(strlen(templ->coinb2) / 2);
	binlify(coinb2, templ->coinb2);
	unsigned char *txmerkles = (unsigned char*)malloc(strlen(templ->txmerkles) / 2);
	binlify(txmerkles, templ->txmerkles);
	unsigned char *version = (unsigned char*)malloc(strlen(templ->version) / 2);
	binlify(version, templ->version);
	unsigned char *nbits = (unsigned char*)malloc(strlen(templ->nbits) / 2);
	binlify(nbits, templ->nbits);
	unsigned char *ntime = (unsigned char*)malloc(strlen(templ->ntime) / 2);
	binlify(ntime, templ->ntime);
	unsigned int truc[1];




	((uint32_t*)version)[0] = bswap32(((uint32_t*)version)[0]);
	((uint32_t*)nbits)[0] = bswap32(((uint32_t*)nbits)[0]);
	((uint32_t*)ntime)[0] = bswap32(((uint32_t*)ntime)[0]);


	for (int i=0;i<strlen(templ->prevhash_be) / 8;i++)
        ((uint32_t*)prevhash_be)[i] = bswap32(((uint32_t*)prevhash_be)[i]);

	truc[0] = job->id;
	json_value *buffer = json_object_new(0);
	json_value * arr = json_array_new(0);
	json_value * arr2 = json_array_new(0);



	if (strlen(templ->txmerkles)>0) {
		int count = 0;
		vector<string>::const_iterator i;
		for (i = templ->txsteps.begin(); i != templ->txsteps.end(); ++i) {
			int len = strlen((*i).c_str());
			debuglog("jobsend txmerkle: transaction len = %d num = %d ", len, count);
			int lenbyte = len / 2;
			unsigned char * bytesstring = (unsigned char*)malloc(lenbyte);
			binlify(bytesstring, (*i).c_str());
			json_array_push(arr2, json_bytes_new(bytesstring, lenbyte));
			free(bytesstring);
			count++;
		}
	}


	json_array_push(arr,json_bytes_new((unsigned char*)truc,4));
	json_array_push(arr,json_bytes_new(prevhash_be,strlen(templ->prevhash_be) / 2));
	json_array_push(arr, json_bytes_new(coinb1,strlen(templ->coinb1) / 2));
	json_array_push(arr, json_bytes_new(coinb2,strlen(templ->coinb2) / 2));


	json_array_push(arr, arr2);
	json_array_push(arr, json_bytes_new(version,strlen(templ->version) / 2));
	json_array_push(arr, json_bytes_new(nbits, strlen(templ->nbits) / 2));
	json_array_push(arr, json_bytes_new(ntime,strlen(templ->ntime) / 2 ));


	json_array_push(arr, json_boolean_new(1));

	json_object_push(buffer, "id", json_null_new());
	json_object_push(buffer, "method", json_string_new("mining.notify"));
	json_object_push(buffer, "params", arr);

	int res = socket_send_raw_mtp(s,buffer);
	
	json_builder_free(buffer);

	free(prevhash_be);
	free(coinb1);
	free(coinb2);
	free(txmerkles);
	free(version);
	free(nbits);
	free(ntime);
	debuglog("******* send new work to miner: result = %d ",res);
	return res;

}

static YAAMP_JOB *job_get_last(int coinid)
{
	g_list_job.Enter();
	for(CLI li = g_list_job.first; li; li = li->prev)
	{
		YAAMP_JOB *job = (YAAMP_JOB *)li->data;
		if(!job_can_mine(job)) continue;
		if(!job->coind) continue;
		if(coinid > 0 && job->coind->id != coinid) continue;

		g_list_job.Leave();
		return job;
	}

	g_list_job.Leave();
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void job_send_last(YAAMP_CLIENT *client)
{
#ifdef NO_EXCHANGE
	// prefer user coin first (if available)
	YAAMP_JOB *job = job_get_last(client->coinid);
	if(!job) job = job_get_last(0);
#else
	YAAMP_JOB *job = job_get_last(0);
#endif
	if(!job) return;

	YAAMP_JOB_TEMPLATE *templ = job->templ;
	client->jobid_sent = job->id;

	if (!strcmp(g_stratum_algo, "mtp")) {
		clientlog(client,"send mining notify message\n");
		job_mining_notify_buffer_mtp_send(client, job, client->sock);
	} else {
    	char buffer[YAAMP_SMALLBUFSIZE];
    	job_mining_notify_buffer(job, buffer);
		socket_send_raw(client->sock, buffer, strlen(buffer));
	}
}

void job_send_jobid(YAAMP_CLIENT *client, int jobid)
{
	YAAMP_JOB *job = (YAAMP_JOB *)object_find(&g_list_job, jobid, true);
	if(!job)
	{
		job_send_last(client);
		return;
	}
	YAAMP_JOB_TEMPLATE *templ = job->templ;
	client->jobid_sent = job->id;

	if (!strcmp(g_stratum_algo, "mtp")) {
		job_mining_notify_buffer_mtp_send(client,job, client->sock);
	}
	else {
    	char buffer[YAAMP_SMALLBUFSIZE];
    	job_mining_notify_buffer(job, buffer);
    	socket_send_raw(client->sock, buffer, strlen(buffer));
	}
	object_unlock(job);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void job_broadcast(YAAMP_JOB *job)
{

printf("\n---------------------------------------JOB BROADCAST -----------------------------\n");
	int s1 = current_timestamp_dms();
	int count = 0;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000; // max time to push to a socket (very fast)

	YAAMP_JOB_TEMPLATE *templ = job->templ;



	g_list_client.Enter();
	for(CLI li = g_list_client.first; li; li = li->next)
	{
		YAAMP_CLIENT *client = (YAAMP_CLIENT *)li->data;
		if(client->deleted) continue;
		if(!client->sock) continue;
	//	if(client->reconnecting && client->locked) continue;

		if(client->jobid_next != job->id) continue;
		if(client->jobid_sent == job->id) continue;

		client->jobid_sent = job->id;
		client_add_job_history(client, job->id);
			if (!strcmp(g_stratum_algo, "mtp"))
					client_adjust_difficulty_mtp(client);
			else
					client_adjust_difficulty(client);

		setsockopt(client->sock->sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

		int result;
		if (!strcmp(g_stratum_algo, "mtp")) {
		    result = job_mining_notify_buffer_mtp_send(client,job, client->sock);
		}
		else {
    		char buffer[YAAMP_SMALLBUFSIZE];
    		job_mining_notify_buffer(job, buffer);
    		result = socket_send_raw(client->sock, buffer, strlen(buffer));
		}
		if (result == -1) {
			int err = errno;
			client->broadcast_timeouts++;
			// too much timeouts, disconnect him
			if (client->broadcast_timeouts >= 3) {
				shutdown(client->sock->sock, SHUT_RDWR);
				clientlog(client, "unable to send job, sock err %d (%d times)", err, client->broadcast_timeouts);
				if(client->workerid && !client->reconnecting) {
				//	CommonLock(&g_db_mutex);
					db_clear_worker(g_db, client);
				//	CommonUnlock(&g_db_mutex);
				}
				object_delete(client);
			}
		}
		count++;
	}

	g_list_client.Leave();
	g_last_broadcasted = time(NULL);

	int s2 = current_timestamp_dms();
	if(!count) return;

	///////////////////////

	uint64_t coin_target = decode_compact(templ->nbits);
	if (templ->nbits && !coin_target)
	    coin_target = 0xFFFF000000000000ULL; // under decode_compact min diff
	double coin_diff = target_to_diff(coin_target);

	debuglog("%s %d - diff %.9f job %x to %d/%d/%d clients, hash %.3f/%.3f in %.1f ms", job->name,
		templ->height, coin_diff, job->id, count, job->count, g_list_client.count, job->speed, job->maxspeed, 0.1*(s2-s1));

//	for(int i=0; i<templ->auxs_size; i++)
//	{
//		if(!templ->auxs[i]) continue;
//		YAAMP_COIND *coind_aux = templ->auxs[i]->coind;
//
//		unsigned char target_aux[1024];
//		binlify(target_aux, coind_aux->aux.target);
//
//		uint64_t coin_target = get_hash_difficulty(target_aux);
//		double coin_diff = target_to_diff(coin_target);
//
//		debuglog("%s %d - diff %.9f chainid %d [%d]", coind_aux->symbol, coind_aux->height, coin_diff,
//				coind_aux->aux.chainid, coind_aux->aux.index);
//	}

}







//	double maxhash = 0;
//	if(job->remote)
//	{
//		sprintf(name, "JOB%d%s (%.3f)", job->remote->id, job->remote->nonce2size == 2? "*": "", job->remote->speed_avg);
//		maxhash = job->remote->speed;
//	}
//	else
//	{
//		strcpy(name, job->coind->symbol);
//		for(int i=0; i<templ->auxs_size; i++)
//		{
//			if(!templ->auxs[i]) continue;
//			YAAMP_COIND *coind_aux = templ->auxs[i]->coind;
//
//			sprintf(name_auxs+strlen(name_auxs), ", %s %d", coind_aux->symbol, templ->auxs[i]->height);
//		}
//
//		maxhash = coind_nethash(job->coind)*coind_profitability(job->coind)/(g_current_algo->profit? g_current_algo->profit: 1);
//	}

