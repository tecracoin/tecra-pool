<?php
JavascriptFile("/extensions/qrcodejs/jquery.min.js");
JavascriptFile("/extensions/qrcodejs/qrcode.js");
JavascriptFile("/extensions/jqplot/jquery.jqplot.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.dateAxisRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.barRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.highlighter.js");
JavascriptFile('/yaamp/ui/js/auto_refresh.js');
$theserver = YAAMP_SITE_URL;
$recents = array();
$raw_recents = isset($_COOKIE['wallets'])? explode("|", $_COOKIE['wallets']): array();
// make it unique
foreach($raw_recents as $addr) {
	$recents[$addr] = $addr;
}

$address = getparam('address');
if (!empty($address) && preg_match('/[^A-Za-z0-9]/', $address)) {
	// Just to make happy XSS seekers who can hack their own browser html...
	die;
}

$drop_address = getparam('drop');
if (!empty($drop_address)) {
	// to clean cookies
	foreach($recents as $k=>$addr) {
		if ($addr == $drop_address) {
			unset($recents[$k]);
			if (controller()->admin)
				setcookie('wallets', implode("|", $recents), time()+60*60*24*30, '/');
			break;
		}
	}
}

$user = getuserparam($address);
if($user)
{
	user()->setState('yaamp-wallet', $user->username);
	$recents[$user->username] = $user->username;

	$coin = getdbo('db_coins', $user->coinid);
	if($coin) echo <<<END
	<script type="text/javascript">
	$(function() {
		$('#favicon').remove();
		$('head').append('<link href="{$coin->image}" id="favicon" rel="shortcut icon">');
	});
	</script>
END;

	if(empty($user->hostaddr) && !$this->admin) {
		$user->hostaddr = $_SERVER['REMOTE_ADDR'];
		$user->save();
	}
}

$username = $user? $user->username: '';

if(!controller()->admin)
	setcookie('wallets', implode("|", $recents), time()+60*60*24*30, '/');



if($user) 
	echo <<<END
	<div class="main-left-box" style='margin-left: 10px; margin-right: 10px; margin-top: 15px;border:0px'>
	<div class="main-left-title" style='font-size:1.5em'><center>Mining summary for $user->username <button id="qrbutton" class="qr_icon"></button></center></div>
	

	  
	<br>
	<div class="main-left-inner">
	<div id="worker_wallet_summary"></div>
	<form action="/" method="get" style="padding: 10px;">
	<div style="float:right">
	<input  type="text" name="address" class="main-text-input" placeholder="New Wallet Address" size="40">
	<!--input  type="submit" value="Submit" class="main-submit-button" -->
	</div><br><br>
	</form>



	<!-- The Modal -->
	<div id="myModal" class="modal">
	
	  <!-- Modal content -->
	  <div class="modal-content">
		<div class="modal-header">
		  <span class="close">&times;</span>
		  <h2>Mining summary on your mobile</h2>
		  <br>
		</div>
		<div class="modal-body">
	<input id="text" type="text" value="http://$theserver/?address=$user->username" style="width:80%;display:none" </input><br>
	<center><div id="qrcode" style="width:160px;height:160px;margin-top:15px;"></div></center>	

		<br><br>
		  <!-- center>http://dev.pool.tecracoin.io/?address=$user->username</center -->
		</div>
		<div class="modal-footer">
		  <center><h4 >http://$theserver/?address=$user->username</h4></center>
		</div>
	  </div>
	
	</div>


	<script type="text/javascript">
	var qrcode = new QRCode(document.getElementById("qrcode"), {
		width : 160,
		height : 160,
		useSVG: true
	});
	function makeCode () {      
		var elText = document.getElementById("text");
		
		if (!elText.value) {
			alert("Input a text");
			elText.focus();
			return;
		}
		
		qrcode.makeCode(elText.value);
	}
	
	makeCode();
	
	$("#text").
		on("blur", function () {
			makeCode();
		}).
		on("keydown", function (e) {
			if (e.keyCode == 13) {
				makeCode();
			}
		});
/////////////////////////////////////////////
// Get the modal
var modal = document.getElementById("myModal");

// Get the button that opens the modal
var btn = document.getElementById("qrbutton");

// Get the <span> element that closes the modal
var span = document.getElementsByClassName("close")[0];

// When the user clicks the button, open the modal 
btn.onclick = function() {
  modal.style.display = "block";
}

// When the user clicks on <span> (x), close the modal
span.onclick = function() {
  modal.style.display = "none";
}

// When the user clicks anywhere outside of the modal, close it
window.onclick = function(event) {
  if (event.target == modal) {
    modal.style.display = "none";
  }
}


    </script>

END;
else 
	echo <<<END
	<div class="main-left-box" style='margin-left: 10px; margin-right: 10px; margin-top: 15px;'>
	<div class="main-left-title">Find miner address:</div>
	<div class="main-left-inner">
	<form action="/" method="get" style="padding: 10px;">
	<input type="text" name="address" class="main-text-input" placeholder="Wallet Address" maxlength="60">
	<!-- input type="submit" value="Submit" class="main-submit-button" --><br><br>
	</form>
END;


/*
echo "<table class='dataGrid2'>";
foreach($recents as $addr)
{
	if(empty($addr)) continue;

	$user = getuserparam($addr);
	if(!$user) continue;

	$coin = getdbo('db_coins', $user->coinid);

	if($user->username == $username)
		echo "<tr style='background-color: #e0d3e8;'><td width=24>";
	else
		echo "<tr class='ssrow'><td width=24>";

	if($coin)
		echo '<img width="16px" src="'.$coin->image.'">';

	echo '</td><td><a class="address" href="/?address='.$addr.'" style="font-family: monospace; font-size: 1.1em;">'.
		$addr.'</a></td>';

	$balance = bitcoinvaluetoa($user->balance);

	if($coin)
		$balance = $balance>0? "$balance $coin->symbol": '';
	else
		$balance = $balance>0? "$balance BTC": '';

	echo '<td align="right">'.$balance.'</td>';
	
	$delicon = $address == $addr ? '' : '<img src="/images/base/delete.png" onclick="javascript:drop_cookie(this);" style="cursor:pointer;"/>';
	echo '<td style="width: 16px; max-width: 16px;">'.$delicon.'</td>';
	
	echo '</tr>';
}
echo "</table></form>
*/
echo "</form></div></div></div>";




//echo "</td><td valign=top>";

echo <<<END
<div id='resume_update_button' style='color: #444; background-color: #ffd; border: 1px solid #eea;
	padding: 10px; margin-left: 20px; margin-right: 20px; margin-top: 15px; cursor: pointer; display: none;'
	onclick='auto_page_resume();' align=center>
	<b>Auto refresh is paused - Click to resume</b></div>
<br>

END;

if($user) echo <<<END
<div style="text-align:center">
<div id='main_graphs_results' class="responsive-div-left">
</div>

END;


if($user) echo <<<END

<div id="zgraph_earning" class="responsive-div-right">
<div class="main-left-box">

<div class="main-left-title">Last 24 Hours Earnings</div>
<div class="main-left-inner"><br>
<div id='graph_earnings_results' style='height: 240px;'></div>
<div style='float: right;'>
<span style='font-size: .8em; color: #4bb2c5;'>Balance</span>
<span style='font-size: .8em; color: #eaa228;'>Pending</span>
</div>
<br>
</div></div><br>
</div></div>
END;


//echo "</td><td valign=top>";
//echo "<table cellspacing=20 width=100%><tr><td valign=top width=50%>";

if($user) echo <<<END
<div style="text-align:center">
<div id='main_miners_results' class="responsive-div-left" style="float:left">

</div>

</div>
END;




//echo "</td><td valign=top>";

if($user) echo <<<END
<div class="responsive-div-right">

<div id='found_results'  >
<!--br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br-->
</div>
END;

if($user) echo <<<END
<div id='main_wallet_results'>
<!--br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br-->
</div>
</div>
<br>
</div>
</div>
END;

echo <<<END

<!--/td></tr></table-->

<!--br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br>
<br><br><br><br><br><br><br><br><br><br-->

<script>

function page_refresh()
{
	pool_current_refresh();
	found_refresh();

	if('$username' != '')
	{
		summary_wallet_refresh();
		main_wallet_refresh();
		main_miners_refresh();

		main_graphs_refresh();
		main_title_refresh();
	}
}

function select_algo(algo)
{
	window.location.href = '/site/algo?algo='+algo+'&r=/site/mining';
}

////////////////////////////////////////////////////

function main_wallet_ready(data)
{
	$('#main_wallet_results').html(data);
}

function main_wallet_refresh()
{
	var url = "/site/wallet_results?address=$username";
	$.get(url, '', main_wallet_ready);
}

function main_wallet_refresh_details()
{
	var url = "/site/wallet_results?address=$username&showdetails=1";
	$.get(url, '', main_wallet_ready);
}

////////////////////////////////////////////////////

////////////////////////////////////////////////////

function summary_wallet_ready(data)
{
	$('#worker_wallet_summary').html(data);
}

function summary_wallet_refresh()
{
	var url = "/site/worker_wallet_summary?address=$username";
	$.get(url, '', summary_wallet_ready);
}



////////////////////////////////////////////////////


function main_miners_ready(data)
{
	$('#main_miners_results').html(data);
}

function main_miners_refresh()
{
	var url = "/site/wallet_miners_results?address=$username";
	$.get(url, '', main_miners_ready);
}

////////////////////////////////////////////////////

function pool_current_ready(data)
{
	$('#pool_current_results').html(data);
}

function pool_current_refresh()
{
	var url = "/site/current_results";
	$.get(url, '', pool_current_ready);
}

////////////////////////////////////////////////////

function main_title_ready(data)
{
	document.title = data;
}

function main_title_refresh()
{
	var url = "/site/title_results?address=$username";
	$.get(url, '', main_title_ready);
}

////////////////////////////////////////////////////

function found_ready(data)
{
	$('#found_results').html(data);
}

function found_refresh()
{
	var url = "/site/user_earning_results?address=$username";
	$.get(url, '', found_ready);
}

////////////////////////////////////////////////////

var last_graph_update = 0;

function main_graphs_ready(data)
{
	$('#main_graphs_results').html(data);
	$('.graph_algo').each(function()
	{
		var algo = $(this).attr('id');
		main_refresh_hashrate(algo);
	});
}

function main_graphs_refresh()
{
	var now = Date.now()/1000;

	if(now < last_graph_update + 900) return;
	last_graph_update = now;

	var url = "/site/wallet_graphs_results?address=$username";
	$.get(url, '', main_graphs_ready);

/*	graph_earnings_refresh(); */

	var url = "/site/graph_earnings_results?address=$username";
	$.get(url, '', graph_earnings_ready);
}

///////////////////////////////////////////////////////////////////////

function main_refresh_hashrate(algo)
{
	var url = "/site/graph_user_results?address=$username&algo="+algo;
	$.get(url, '', function(data)
	{
		graph_init_hashrate(data, algo);
	});
}

///////////////////////////////////////////////////////////////////////

function graph_init_hashrate(data, algo)
{
	$('#graph_results_'+algo).empty();

	var t = $.parseJSON(data);
	var plot1 = $.jqplot('graph_results_'+algo, t,
	{
		title: '<b>'+algo+' Hashrate (Mh/s)</b>',
		axes: {
			xaxis: {
				tickInterval: 7200,
				renderer: $.jqplot.DateAxisRenderer,
				tickOptions: {formatString: '<font size=1>%#Hh</font>'},
				color:'white'
			},
			yaxis: {
				min: 0.0,
				tickOptions: {formatString: '<font size=1>%#.3f &nbsp;</font>'}
			}
		},
		/*
		seriesDefaults:
		{
		markerOptions: { style: 'none' } 
		},
		*/
		series: 
		[ {
			color: "#C35F08",
/*			highlightColors: ['white'], */
			fill: true			
		}, 
		{
			color: "#328ba8", 
/*			highlightColors: ['white','lightpink', 'lightsalmon'],*/
			markerOptions: { style: 'none' }
		}],
		
		grid:
		{
			borderWidth: 0.5,
			shadowWidth: 0,
			shadowDepth: 0,
			background: '#0',
			gridLineWidth: 0.25,
			gridLineColor: 'grey'
		},
		highlighter:
		{
			show: true,
			useAxesFormatters: false,
			tooltipAxes: 'y',
			tooltipFormatString: '%s Mh/s'
		},
		cursor: {
			show: false,
			tooltipLocation:'sw'
		  }

	});
	$('.jqplot-highlighter-tooltip').css({color:'rgb(192, 189, 189)','background': 'rgb(71, 71, 71)', 'padding': '2px 5px'});
	$(window).resize(function() {
		plot1.replot( { resetAxes: true ,axes:{yaxis:{min:0}}}  );
  });

}

///////////////////////////////////////////////////////////////////////

function graph_earnings_ready(data)
{
	graph_earnings_init(data);
}

function graph_earnings_refresh()
{
	var url = "/site/graph_earnings_results?address=$username";
	$.get(url, '', graph_earnings_ready);
}

function graph_earnings_init(data)
{
	$('#graph_earnings_results').empty();

	var t = $.parseJSON(data);
	var plot1 = $.jqplot('graph_earnings_results', t,
	{
	//	title: '<b></b>',
		title: '<b> Unpaid balance </b>',
		stackSeries: true,
		axes: {
			xaxis: {
				tickInterval: 7200,
				renderer: $.jqplot.DateAxisRenderer,
				tickOptions: {formatString: '<font size=1>%#Hh</font>'}
			},
			yaxis: {
				min: 0,
				tickOptions: {formatString: '<font size=1>%#.8f &nbsp;</font>'}
			}
		},

		seriesDefaults:
		{
			markerOptions: { style: 'none' }
		},

		grid:
		{
			borderWidth: 0.5,
			shadowWidth: 0,
			shadowDepth: 0,
			background: '#0',
			gridLineWidth: 0.25,
			gridLineColor: 'grey'
		}

	});
	$(window).resize(function() {
		plot1.replot( { resetAxes: true ,axes:{yaxis:{min:0}}}  );});
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

function main_wallet_tx()
{
	var w = window.open("/site/tx?address=$username", "yaamp_tx",
		"width=800,height=600,location=no,menubar=no,resizable=yes,status=yes,toolbar=yes");
}

function drop_cookie(el)
{
	var addr = $(el).closest('tr').find('td a.address').text();
	window.location.href = '?address={$address}&drop=' + addr;
}

</script>


END;

