<?php

function WriteBoxHeader($title)
{
/*
	echo "<div class='main-left-box'>";
	echo "<div class='main-left-title'>$title</div>";
	echo "<div class='main-left-inner'>";
*/	
}

function WriteStats($title,$value,$margin)
{

echo <<<TRUC
<div  class='main-left-box' style="display:inline-block;margin: $margin;">
<div class='main-left-title' style="font-size:2.0em">$title</div>
<div class='main-left-inner'>
<br>
<span style="font-weight:bold;font-size:2.0em">$value</span><br>
<br>
</div></div>
TRUC;
}


$mining = getdbosql('db_mining');
$defaultalgo = user()->getState('yaamp-algo');

$show_details = getparam('showdetails');

$user = getuserparam(getparam('address'));
if(!$user) return;

WriteBoxHeader("bla bla");

$refcoin = getdbo('db_coins', $user->coinid);
if(!$refcoin)
{
	if($user->coinid != null)
		echo "<div style='color: red; padding: 10px; '>This wallet address is not valid.
			You will not receive payments using this address.</div>";

	$refcoin = getdbosql('db_coins', "symbol='BTC'");

} elseif (!YAAMP_ALLOW_EXCHANGE && $user->coinid == 6 && $defaultalgo != 'sha256') {

	echo "<div style='color: red; padding: 10px; '>This pool does not convert/trade currencies.
		You will not receive payments using this BTC address.</div>";
	return;
}

$total_pending = 0;

//////////////////////////////////////////////////////////////////////////////
$confirmed = yaamp_convert_earnings_user($user, "status=1");
$unconfirmed = yaamp_convert_earnings_user($user, "status=0");

$total_unsold = bitcoinvaluetoa($confirmed + $unconfirmed);
$confirmed = $confirmed? bitcoinvaluetoa($confirmed): '';
$unconfirmed = /*$unconfirmed?*/ bitcoinvaluetoa($unconfirmed) /*: ''*/;
//$total_usd = number_format($total_unsold*$mining->usdbtc*$refcoin->price, 3, '.', ' ');
$total_pending = bitcoinvaluetoa($total_pending);

$balance = bitcoinvaluetoa($user->balance);
$balance2 = bitcoinvaluetoa($user->balance+$confirmed);

$total_unpaid = bitcoinvaluetoa($balance + $total_unsold);

$total_paid = controller()->memcache->get_database_scalar("wallet_total_paid-$user->id",
	"select sum(amount) from payouts where account_id=$user->id");

$total_paid = bitcoinvaluetoa($total_paid);
//$total_paid_usd = number_format($total_paid*$mining->usdbtc*$refcoin->price, 3, '.', ' ');

$total_earned = bitcoinvaluetoa($total_unsold + $balance + $total_paid);
//$total_earned_usd = number_format($total_earned*$mining->usdbtc*$refcoin->price, 3, '.', ' ');

$userid = intval($user->id);
$coinid = intval($user->coinid);
if ($coinid) {
	$coin = getdbo('db_coins', $coinid);
}

/////////////////////////////////////////////////////////////////////
//foreach(yaamp_get_algos() as $algo)
$algo = 'mtp';

{
	if (!YAAMP_ALLOW_EXCHANGE && isset($coin) && $coin->algo != $algo) return;

	$user_rate1 = yaamp_user_rate($userid, $algo);
	$user_rate1_bad = yaamp_user_rate_bad($userid, $algo);

	$percent_bad = ($user_rate1 + $user_rate1_bad)? $user_rate1_bad * 100 / ($user_rate1 + $user_rate1_bad): 0;
	$percent_bad = $percent_bad? round($percent_bad, 1).'%': '';

	$user_rate1 = $user_rate1? Itoa2($user_rate1).'h/s': '-';
	$minercount = getdbocount('db_workers', "userid=$userid AND algo=:algo", array(':algo'=>$algo));

	if (YAAMP_ALLOW_EXCHANGE || !$user->coinid) {

		$user_shares = controller()->memcache->get_database_scalar("wallet_user_shares-$userid-$algo",
			"SELECT SUM(difficulty) FROM shares WHERE valid AND userid=$userid AND algo=:algo", array(':algo'=>$algo));


		$total_shares = controller()->memcache->get_database_scalar("wallet_total_shares-$algo",
			"SELECT SUM(difficulty) FROM shares WHERE valid AND algo=:algo", array(':algo'=>$algo));

	} else {
		// we know the single currency mined if auto exchange is disabled
		$user_shares = controller()->memcache->get_database_scalar("wallet_user_shares-$algo-$coinid-$userid",
			"SELECT SUM(difficulty) FROM shares WHERE valid AND userid=$userid AND coinid=$coinid AND algo=:algo", array(':algo'=>$algo));


		$total_shares = controller()->memcache->get_database_scalar("wallet_coin_shares-$coinid",
			"SELECT SUM(difficulty) FROM shares WHERE valid AND coinid=$coinid AND algo=:algo", array(':algo'=>$algo));
	}

	if(!$total_shares) return;
	$percent_shares = round($user_shares * 100 / $total_shares, 4);

}


echo<<<NEWLAYOUT
<div     style="text-align:center";>
<br>
NEWLAYOUT;

echo "<div style='margin:20px; display:inline-block'>";

if ($minercount==0)
WriteStats("Total Hashrate",$user_rate1,'1px');
else if ($minercount==1)
WriteStats("Total Hashrate",$user_rate1.'  ('.$minercount.' worker)','1px' );
else 
WriteStats("Total Hashrate",$user_rate1.'  ('.$minercount.' workers)','1px' );

if ($percent_bad)
WriteStats("%age rejected",$percent_bad,'1px');

WriteStats("Round share",$percent_shares.'%','1px');

echo "</div>";

//echo "<div style='margin:20px; display:inline-block'></div>";
echo "<div style='margin:20px; display:inline-block'>";
WriteStats("Unconfirmed",($unconfirmed>0.)?round($unconfirmed,3).' '.$refcoin->symbol2:'-','2px');
WriteStats("Unpaid Balance",($balance2>0.)?round($balance2,3).' '.$refcoin->symbol2:'-','1px');
WriteStats("Total Paid",($total_paid>0.)?round($total_paid,3).' '.$refcoin->symbol2:'-','1px');
echo "</div>";

echo<<<NEWLAYOUT
</div>
NEWLAYOUT;


////////////////////////////////////////////////////////////////////////////////



return;






