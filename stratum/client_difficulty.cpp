
#include "stratum.h"

double client_normalize_difficulty(double difficulty)
{
	if(difficulty < g_stratum_min_diff) difficulty = g_stratum_min_diff;
	else if(difficulty < 1) difficulty = floor(difficulty*1000/2)/1000*2;
	else if(difficulty > 1) difficulty = floor(difficulty/2)*2;
	if(difficulty > g_stratum_max_diff) difficulty = g_stratum_max_diff;
	return difficulty;
}

void client_record_difficulty_old(YAAMP_CLIENT *client)
{
	if(client->difficulty_remote)
	{
		client->last_submit_time = current_timestamp();
		return;
	}

	int e = current_timestamp() - client->last_submit_time;
	if(e < 1000) e = 1000;
	int p = 10;

	client->shares_per_minute = (client->shares_per_minute * (100 - p) + 60*2000*p/e) / 100;

	client->last_submit_time = current_timestamp();

}

void client_record_difficulty(YAAMP_CLIENT *client)
{
/// calculate the average time between shares as it should be done 
	if (client->difficulty_remote)
	{
		client->last_submit_time = current_timestamp();
		return;
	}
    client->Ndelta++;
	uint32_t ts = current_timestamp(); 
    uint32_t sinceLast = ts - client->last_submit_time;
	client->dtime_shares += sinceLast;
	client->avg_timeshare = (double)(client->dtime_shares)/(double)(client->Ndelta*1000); // shares/sec
	client->shares_per_minute = (client->avg_timeshare==0.)? 10.0 : 60.0/client->avg_timeshare;
	client->last_submit_time = ts;

        clientlog(client,"share per minutes = %f \n",client->shares_per_minute);

}



void client_change_difficulty(YAAMP_CLIENT *client, double difficulty)
{
	if(difficulty <= 0) return;

	difficulty = client_normalize_difficulty(difficulty);
	if(difficulty <= 0) return;

//	debuglog("change diff to %f %f", difficulty, client->difficulty_actual);
	if(difficulty == client->difficulty_actual) return;

	uint64_t user_target = diff_to_target(difficulty);
	if(user_target >= YAAMP_MINDIFF && user_target <= YAAMP_MAXDIFF)
	{
		client->difficulty_actual = difficulty;
		client_send_difficulty(client, difficulty);
	}
}



void client_change_difficulty_mtp(YAAMP_CLIENT *client, double difficulty)
{

	if (difficulty <= 0) return;

	difficulty = client_normalize_difficulty(difficulty);
	if (difficulty <= 0) return;

	if (difficulty<g_stratum_difficulty)
				difficulty=g_stratum_difficulty; // the difficulty can't be smaller than min diff

	if (difficulty == client->difficulty_actual) return;

	uint64_t user_target = diff_to_target(difficulty);

if (client->difficulty_min) {
	uint64_t minimal_target = diff_to_target(client->difficulty_minimal);

//	if (user_target >= minimal_target /*&& user_target <= YAAMP_MAXDIFF*/)
	if(difficulty >= client->difficulty_minimal && user_target <= YAAMP_MAXDIFF)
	{
		client->difficulty_actual = difficulty;
		client_send_target_mtp(client, difficulty);
	}
}
else {
	if (user_target >= YAAMP_MINDIFF && user_target <= YAAMP_MAXDIFF)
	{
		client->difficulty_actual = difficulty;
		client_send_target_mtp(client, difficulty);
	}
}
}


void client_adjust_difficulty(YAAMP_CLIENT *client)
{
	if(client->difficulty_remote) {
		client_change_difficulty(client, client->difficulty_remote);
		return;
	}

	if(client->shares_per_minute > 100)
		client_change_difficulty(client, client->difficulty_actual*4);

	else if(client->difficulty_fixed)
		return;

	else if(client->shares_per_minute > 25)
		client_change_difficulty(client, client->difficulty_actual*2);

	else if(client->shares_per_minute > 20)
		client_change_difficulty(client, client->difficulty_actual*1.5);

	else if(client->shares_per_minute <  5)
		client_change_difficulty(client, client->difficulty_actual/2);
}


void client_adjust_difficulty_mtp(YAAMP_CLIENT *client)
{
	clientlog(client, "######## client->shares_per_minute = %f fixed = %d minimal = %d\n", client->shares_per_minute, client->difficulty_fixed, client->difficulty_min);
	if (client->difficulty_remote) {
		client_change_difficulty_mtp(client, client->difficulty_remote);
		return;
	}
	uint32_t ts = current_timestamp();
	if (!client->lastRtc)
	{
	client->lastRtc = ts - ((uint32_t)(g_stratum_vardiff_retargettime))/2000;
	}
	double variance_plus = ((double)(g_stratum_vardiff_targettime))*g_stratum_vardiff_varplus; // 30% variance between 2 and 4 shares per minutes
	double variance_minus = ((double)(g_stratum_vardiff_targettime))*g_stratum_vardiff_varmin; // 40% variance between 2 and 4 shares per minutes
	double tMin = ((double)(g_stratum_vardiff_targettime)) - variance_minus;
	double tMax = ((double)(g_stratum_vardiff_targettime)) + variance_plus;

	if ((ts-client->lastRtc)<(g_stratum_vardiff_retargettime*1000))
		return;

	if (client->difficulty_fixed)
		return;

	client->lastRtc = ts;

	double avg = client->avg_timeshare;
	double ddiff = (avg>0.)? ((double)(g_stratum_vardiff_targettime))/avg : 0.25;

	if (avg > tMax)
	{
        client->dtime_shares = 0; // we reset after the difficulty is adjusted
        client->Ndelta = 0;
        client->avg_timeshare=0;
        client->shares_per_minute=0;
		client_change_difficulty_mtp(client, client->difficulty_actual * ddiff);
	}
	else if (avg < tMin)
	{
        client->dtime_shares = 0; // we reset after the difficulty is adjusted
        client->Ndelta = 0;
        client->avg_timeshare=0;
        client->shares_per_minute=0;
		client_change_difficulty_mtp(client, client->difficulty_actual * ddiff);
	}
	else 
		return;
}


int client_send_difficulty(YAAMP_CLIENT *client, double difficulty)
{
//	debuglog("%s diff %f", client->sock->ip, difficulty);
	client->shares_per_minute = YAAMP_SHAREPERSEC;

	if(difficulty >= 1)
		client_call(client, "mining.set_difficulty", "[%.0f]", difficulty);
	else
		client_call(client, "mining.set_difficulty", "[%.3f]", difficulty);
	return 0;
}

int client_send_target_mtp(YAAMP_CLIENT *client, double difficulty)
{
	//	debuglog("%s diff %f", client->sock->ip, difficulty);
	client->shares_per_minute = YAAMP_SHAREPERSEC;
	uint32_t target[8];
	
	diff_to_target(target, difficulty);

	json_value *buffer = json_object_new(0);
	json_value * arr = json_array_new(0);
	json_array_push(arr, json_bytes_new((uint8_t*)target,32));
	json_object_push(buffer, "id", json_null_new());
	json_object_push(buffer, "method", json_string_new("mining.set_target"));
	json_object_push(buffer, "params", arr);

	int res = socket_send_raw_mtp(client->sock, buffer);

//	json_builder_free(buffer);
	json_builder_free(buffer);

//		client_call_mtp(client, "mining.set_target", "[%s]", cTarget);
	return 0;
}

void client_initialize_difficulty(YAAMP_CLIENT *client)
{


	char *p = strstr(client->password, "d=");  // starting difficulty => the pool adjust difficulty in order to reach plan goal of 2 shares/minutes
//	char *p2 = strstr(client->password, "fd="); // fixed difficulty=> vardiff is desactived (unless it is well below plan goal
	char *p3 = strstr(client->password, "md="); // minimal difficulty=> vardiff can only increase difficulty to reach plan goal
//	if(!p && !p2 && !p3) return;
	client->dtime_shares = 0;
	client->Ndelta = 0;
	client->start_time = current_timestamp();
//	client->last_submit_time = current_timestamp();
// starting difficulty 
	if (p && !p3) {
		double diff = client_normalize_difficulty(atof(p+2));

		uint64_t user_target = diff_to_target(diff);

		if (diff<=g_stratum_difficulty)
			client->difficulty_actual = g_stratum_difficulty;
		else 
			client->difficulty_actual = diff;
	}

// fixed difficulty
/*
	if (p2 && !p3) {
		double diff2 = client_normalize_difficulty(atof(p2 + 3));

		uint64_t user_target2 = diff_to_target(diff2);

		if (diff2>= g_stratum_difficulty)
		{
			client->difficulty_actual = diff2;
			client->difficulty_fixed = true;
		} else {
			client->difficulty_actual = g_stratum_difficulty;
			client->difficulty_fixed = true;			
		}
	}
*/
// minimal difficulty
	if (p3 && !p) {
		double diff3 = client_normalize_difficulty(atof(p3 + 3));
		uint64_t user_target3 = diff_to_target(diff3);

		if(diff3>=g_stratum_difficulty)
		{
			client->difficulty_actual  = diff3;
			client->difficulty_minimal = diff3;
			client->difficulty_min = true;
		
		} else {
			client->difficulty_actual  = g_stratum_difficulty;
			client->difficulty_minimal = g_stratum_difficulty;
			client->difficulty_min = true;			
		}
	}


}



