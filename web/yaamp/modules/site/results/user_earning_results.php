<?php

function WriteBoxHeader($title)
{
/*	
	echo "<div class='main-left-box'>";
	echo "<div class='main-left-title'>$title</div>";
	echo "<div class='main-left-inner'>";
*/	

	echo "<div class='main-left-box'>";
	echo "<div class='main-left-title' id='main-left-title'>$title <span style='float:right'><strong>(expand)</strong></span></div>";
	echo "<br>";
	echo "<div class='main-left-inner' id='detail_div' style='display:inline;vertical-align:super'></div>";
	echo "<div class='main-left-inner' id='main-left-inner' style='display:none'>";

}

$algo = user()->getState('yaamp-algo');

$user = getuserparam(getparam('address'));
if(!$user || $user->is_locked) return;

$count = getparam('count');
$count = $count? $count: 50;
$count = 5000000000;
$day = 24;
$t2 = time()- 60*60*$day;
//WriteBoxHeader("Last $count Earnings");
$zTitle ="Last $day hours Earnings";
WriteBoxHeader($zTitle);
$earnings = getdbolist('db_earnings', "userid=$user->id AND create_time>$t2 order by create_time desc "); //, array(':count'=>$count));
//$earnings = getdbolist('db_earnings', "userid=$user->id order by create_time desc limit :count", array(':count'=>$count));
echo <<<EOT

<style type="text/css">
span.block { padding: 2px; display: inline-block; text-align: center; min-width: 75px; border-radius: 3px; }
span.block.invalid  { color: white; background-color: #d9534f; }
span.block.immature { color: white; background-color: #f0ad4e; }
span.block.exchange { color: white; background-color: #5cb85c; }
span.block.cleared  { color: white; background-color: gray; }
</style>
<br>
<table class="dataGrid2">
<thead>
<tr>
<td></td>
<th>Name</th>
<th align=right>Amount</th>
<th align=right>Percent</th>
<th align=right>mBTC</th>
<th align=right>Time</th>
<th align=right>Status</th>
</tr>
</thead>

EOT;

$showrental = (bool) YAAMP_RENTAL;
$total_earning = 0;
$total_earning_c = 0;
$total_earning_u = 0;

foreach($earnings as $earning)
{
	$coin = getdbo('db_coins', $earning->coinid);
	$block = getdbo('db_blocks', $earning->blockid);
	if (!$block) {
		debuglog("missing block id {$earning->blockid}!");
		continue;
	}

	$d = datetoa2($earning->create_time);

	
	if(!$coin)
	{
		if (!$showrental)
			continue;

		$reward = bitcoinvaluetoa($earning->amount);
		$value = mbitcoinvaluetoa($earning->amount*1000);
		$percent = $block->amount ? percentvaluetoa($earning->amount * 100/$block->amount) : 0;
		$total_earning += $reward;
		if ($earning->status==0)
		$total_earning_u += bitcoinvaluetoa($earning->amount);
		else if ($earning->status>=1)
		$total_earning_c += bitcoinvaluetoa($earning->amount);

		echo '<tr class="ssrow" >';
		echo '<td width="18"><img width="16" src="/images/btc.png"></td>';
		echo '<td><b>Rental</b><span style="font-size: .8em;"> ('.$block->algo.')</span></td>';
		echo '<td align="right" style="font-size: .8em;"><b>'.$reward.' BTC</b></td>';
		echo '<td align="right" style="font-size: .8em;">'.$percent.'%</td>';
		echo '<td align="right" style="font-size: .8em;">'.$value.'</td>';
		echo '<td align="right" style="font-size: .8em;">'.$d.'&nbsp;ago</td>';
		echo '<td align="right" style="font-size: .8em;"><span class="block cleared">Cleared</span></td>';
		echo '</tr>';

		continue;
	}

	$reward = altcoinvaluetoa($earning->amount);
	$percent = $block->amount ? percentvaluetoa($earning->amount * 100/$block->amount) : 0;
	$value = mbitcoinvaluetoa($earning->amount*$earning->price*1000);
	$total_earning += $reward;

	if ($earning->status==0)
	$total_earning_u += bitcoinvaluetoa($earning->amount);
	else if ($earning->status>=1)
	$total_earning_c += bitcoinvaluetoa($earning->amount);

	$blockUrl = $coin->createExplorerLink($coin->name, array('height'=>$block->height));
	
	echo '<tr class="ssrow" >';
	echo '<td width="18"><img width="16" src="'.$coin->image.'"></td>';
	echo '<td><b>'.$blockUrl.'</b><span style="font-size: .8em;"> ('.$coin->algo.')</span></td>';
	echo '<td align="right" style="font-size: .8em;"><b>'.$reward.' '.$coin->symbol_show.'</b></td>';
	echo '<td align="right" style="font-size: .8em;">'.$percent.'%</td>';
	echo '<td align="right" style="font-size: .8em;">'.$value.'</td>';
	echo '<td align="right" style="font-size: .8em;">'.$d.'&nbsp;ago</td>';
	echo '<td align="right" style="font-size: .8em;">';
	
	if($earning->status == 0) {
		$eta = '';
		if ($coin->block_time && $coin->mature_blocks) {
			$t = (int) ($coin->mature_blocks - $block->confirmations) * $coin->block_time;
			$eta = "ETA: ".sprintf('%dh %02dmn', ($t/3600), ($t/60)%60);
		}
		echo '<span class="block immature" title="'.$eta.'">Immature ('.$block->confirmations.')</span>';
	}

	else if($earning->status == 1)
		echo '<span class="block exchange">'.(YAAMP_ALLOW_EXCHANGE ? 'Exchange' : 'Confirmed').'</span>';

	else if($earning->status == 2)
		echo '<span class="block cleared">Cleared</span>';

	else if($earning->status == -1)
		echo '<span class="block invalid">Invalid</span>';

	echo "</td>";
	echo "</tr>";

}

echo "</table>";
echo "<br>";

echo "<br></div></div><br>";
echo "<div id='total_earning' style='display:none' >$total_earning</div>";
echo "<div id='total_earning_c' style='display:none' >$total_earning_c</div>";
echo "<div id='total_earning_u' style='display:none' >$total_earning_u</div>";
echo <<<EOT

<script>
var coll = document.getElementById("main-left-title");

var tot = document.getElementById("total_earning").textContent;
var tot_c = document.getElementById("total_earning_c").textContent;
var tot_u = document.getElementById("total_earning_u").textContent;

var summary = document.getElementById("detail_div");
	summary.innerHTML = '<b>Total: '+tot+' TCR (confirmed '+tot_c+' unconfirmed '+tot_u+') </b>';

  coll.addEventListener("click", function() {
    this.classList.toggle("active");
	  var content = document.getElementById("main-left-inner");

    if (content.style.display === "inline") {
		this.innerHTML = "$zTitle <span style='float:right'><strong>(expand)</strong></span>";
	  content.style.display = "none";
    } else {
	  content.style.display = "inline";
	  this.innerHTML = "$zTitle <span style='float:right;'><strong>(collapse)</strong></span>";
    }
  });

</script>

EOT;
