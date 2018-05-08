var sdDataPos=0;  //视频标清播放焦点
var hdDataPos=0;  //视频标清播放焦点
var msgKey="";
var statationkey="";
var sdArrary=[];
var positionArray=[];
var ext=".jpg";
//var ext=".png"
var datetimer;
function getdocument() {
	ajax();
}

var weekArray = [
                 {"zh_name":"星期日","en_name":"Sunday"},
                 {"zh_name":"星期一","en_name":"Monday"},
                 {"zh_name":"星期二","en_name":"Tuesday"},
                 {"zh_name":"星期三","en_name":"Wednesday"},
                 {"zh_name":"星期四","en_name":"Thursday"},
                 {"zh_name":"星期五","en_name":"Friday"},
                 {"zh_name":"星期六","en_name":"Saturday"}
                 ];

function ajax_xmlhttp() {
	var XmlHttp;
	if (window.ActiveXObject) {
		var arr = ["MSXML2.XMLHttp.6.0", "MSXML2.XMLHttp.5.0",
				"MSXML2.XMLHttp.4.0", "MSXML2.XMLHttp.3.0", "MSXML2.XMLHttp",
				"Microsoft.XMLHttp"];
		for (var i = 0; i < arr.length; i++) {
			try {
				XmlHttp = new ActiveXObject(arr[i]);
				return XmlHttp;
			} catch (error) {

			}
		}
	} else {
		try {
			XmlHttp = new XMLHttpRequest();
			return XmlHttp;
		} catch (otherError) {

		}
	}
}

function ajax() {
	var jsonData;
	var url = "index.json";
	var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
	ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
	ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
		if (ajax.readyState == 4) {// 数据返回成功
			jsonData = ajax.responseText;
//			jsonData = jsonData.replace(/\s+/g, "");
			var page = eval('(' + jsonData + ')');
			creteDocument(page);
		}
	}
	ajax.send(null);

}
/*function station_ajax() {
	// station_direction,station_station
	var jsonData;
	var url =station_url;
	var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
	ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
	ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
		if (ajax.readyState == 4) {// 数据返回成功
			jsonData = ajax.responseText;
			//jsonData = jsonData.replace(/\s+/g, "");
			var progrom = eval('(' + jsonData + ')');
			var directions=progrom["direction"];
			var dire=directions.split("|");
			$("#station_direction").html(dire[0]+"<br>"+dire[1]);
			var flags=progrom["flag"];
			var fla=flags.split("|");
			var stations=progrom["station"];
			var stat=stations.split("|");
			$("#station_station").html(fla[0]+"<br>"+fla[1]+"<br><br>"+stat[0]+"<br><span style='font-size:24px'>"+stat[1]+"</span>");
		}
	}
	ajax.send(null);

}

//ajax图片显示报站信息
function station_ajax_photo() {
	// station_direction,station_station
	var jsonData;
//	station_url="station.json";
	var url =station_url;
	var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
	ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
	ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
		if (ajax.readyState == 4) {// 数据返回成功
			jsonData = ajax.responseText;
			//jsonData = jsonData.replace(/\s+/g, "");
			var progrom = eval('(' + jsonData + ')');
			if(!progrom)
				return;
			 var id=progrom.id;
			 if(statationkey!=id){
				var starts=progrom.start;
				var start=document.getElementById("station_start");
				if(start && start!=null)
					start.src="../station/"+starts;
				var ends=progrom.end;
				var end=document.getElementById("station_end");
				if(end && end!=null)
					end.src="../station/"+ends;
				var stations=progrom.station;
				var sta=document.getElementById("station_station");
				if(sta && sta!=null)
					sta.src="../station/"+stations;
				statationkey=id
			 }
		}
	}
	ajax.send(null);

}*/
/**
 * 事件通知方式显示到站信息
 * @param {} jsonData
 */
//var str="{'type':'EVENT_REPORT_STATION','message':\"{'id':'0123444983e24','line':'1','start':'1','end':'30','type':'1','station':'"+ii+"','doorside':'0'}\"}";
function updatestation(jsonData) {
	var progrom = eval('(' + jsonData + ')');
	if(!progrom)
		return;
	 var id=progrom.id;
	// alert("id=="+id+" start="+progrom.start+" end="+progrom.end+" station="+progrom.station);
	 //alert("start.src="+"../picture/s"+starts+ext+" end.src="+"../picture/e"+ends+ext+" sta.src="+"../picture/"+type+"_"+stations+ext);
	 if(statationkey!=id){
		var starts=progrom.start*1;
		var start=document.getElementById("station_start");
		if(start && start!=null)
			start.src="../station/s"+starts+ext;
		var ends=progrom.end*1;
		var end=document.getElementById("station_end");
		if(end && end!=null)
			end.src="../station/e"+ends+ext;
			
		var type=progrom.type;
		var stations=progrom.station*1;
		var sta=document.getElementById("station_station");
		if(sta && sta!=null)
			sta.src="../station/"+type+"_"+stations+ext;
		
		var current=document.getElementById("station_current");
		var next=document.getElementById("station_next");
		if(type== 1){
			
			if(current && current!=null){
				current.src= "../station/1_"+stations+ext;
			}
			if(next && next!=null){
				if(starts< ends){
					if(stations+1> ends){
						next.src= "../station/empty_station.jpg";  //下一站的 站号超过 end时，下一站 贴一张 empty_station.jpg
					}else{
						next.src= "../station/2_"+(stations+1)+ext;
					}
					
				}else{ //当  反向行驶时，  例如 26 开往  1，      
					if(stations-1< ends){ // 当 当前站=1时   下一站=2
//						next.src= "../picture/2_"+(stations+1)+ext;
						next.src= "../station/empty_station.jpg";
						
					}else{ //当 当前站=3时   下一站=2， 当 当前站=2时  下一站=1，
						next.src= "../station/2_"+(stations-1)+ext;
					}
				}
			}
		}else if(type== 2){
			if(next && next!=null){
				next.src= "../station/2_"+stations+ext;
			}
			if(current && current!=null){   
				if(starts< ends){
					current.src= "../station/1_"+(stations-1)+ext;
				}else{
					if(stations== starts){
						current.src= "../station/1_"+stations+ext;
					}else{
						current.src= "../station/1_"+(stations+1)+ext;
					}
					
				} 
			}
		}
		
		statationkey=id
	 }
			 

}

/*function msg_ajax() {
	var jsonData;
	var url =msg_url;
	var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
	ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
	ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
		if (ajax.readyState == 4) {// 数据返回成功
			jsonData = ajax.responseText;
			//jsonData = jsonData.replace(/\s+/g, "");
			var progrom = eval('(' + jsonData + ')');
			if(progrom){
				if(msgKey!=progrom.key){
					$("#msg").html("<marquee behavior='scroll'  scrollamount='5'  direction='left' width='100%' style='color:"+progrom.fontcolor+";font-size:"+progrom.fontsize+"px;background:"+progrom.bgcolor+";'>"+progrom.msg+"</marquee>");
					msgKey=progrom.key;
					setTimeout(function(){
					$("#msg").html("");
				}, parseInt(progrom.timer)*1000+2000);//有延迟2-3s
				}
			}
//			$("#station").html("当前站:"+progrom["current"]+"<br>下一站:"+progrom["next"])
		}
	}
	ajax.send(null);

}*/


//播放类型
/*function playtype_ajax() {
//	var playtype="";
	var url =playtype_url;
	var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
	ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
	ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
		if (ajax.readyState == 4) {// 数据返回成功
			var type = ajax.responseText;
			if(type=="11"){
				playtype="直播（主）";
			}else if(type=="12"){
				playtype="录播（主）";
			}else if(type=="13"){
				playtype="紧急播放（主）";
			}else if(type=="01"){
				playtype="直播（备）";
			}else if(type=="02"){
				playtype="录播（备）";
			}else if(type=="03"){
				playtype="紧急播放（备）";
			}else if(type=="00"){
				playtype="备";
			}else if(type=="10"){
				playtype="主";
			}else{
				playtype="";
			}
//			if(type=="00" || type=="10")
//				getDiv(0,0,playtype);
//			else
//				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);
			getDiv(0,0, playtype);
			
			
			
			
			
			switch (type){
			case "11":
				playtype="直播（主）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "12":
				playtype="录播（主）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "13":
				playtype="紧急播放（主）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "01":
				playtype="直播（备）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "02":
				playtype="录播（备）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "03":
				playtype="紧急播放（备）";
				getDiv(positionArray[0] + positionArray[2] - 300,positionArray[1], playtype);	
				break;
			case "00":
				playtype="备";
				getDiv(0,0 ,playtype);	
				break;
			case "10":
				playtype="主";
				getDiv(0,0 ,playtype);	
				break;
				
		
			
				
			}
	
		}
	}
	ajax.send(null);

}*/





function creteDocument(page) {

	var regions = page.regions;
	for (var i = 0; i < regions.length; i++) {

		var region = regions[i];
		var div = document.createElement("div");
		div.id = region.id;
		div.style.zIndex = region.zindex;
		div.style.position = "absolute";
		div.style.left = region.x;
		div.style.top = region.y;
		div.style.width = region.width;
		div.style.height = region.height;
		div.style.backgroundColor = region.bgcolor == ""? "transparent": region.bgcolor;
		// div.style.cursor = "pointer";
		if (region.xtype == "img") {
			var imgs = region.imgs;
			var timer = 5;
			//for (var j = 0; j < imgs.length; j++)
			{
				
				var img = document.createElement("img");
				img.id = "content_" + region.id;
				img.style.position = "absolute";
				img.style.width = "100%";
				img.style.height = "100%";
				img.style.border = 0;
				img.src = imgs[0].src;
				timer = imgs[0].timer;
				div.appendChild(img);
			}
			document.body.appendChild(div);
			imgInterval(region);
			/**
			$(window).ready(function() {
						$("#" + region.id).cycle({
									fx : 'cover',
									timeout : timer*1000,
									speed : 500,
									// width:320,
									// height:200,
//									pager : '#nav',
									pause : 1
								});
					});
					**/
			
					
					
		}else if (region.xtype == "background") {
			var src = region.src;
			
				
			var img = document.createElement("img");
			img.id = "content_" + region.id;
			img.style.position = "absolute";
			img.style.width = "100%";
			img.style.height = "100%";
			img.style.border = 0;
			img.src = src;
			
			div.appendChild(img);
			
			document.body.appendChild(div);
			
					
					
		}else if(region.xtype == "word"){
				var wordattr=region.word;
				var word = document.createElement("marquee");
				word.setAttribute("id","content_" + region.id);
				word.setAttribute("behavior","scroll");
				word.setAttribute("scrollamount",wordattr.speed);
				word.setAttribute("direction",wordattr.showmode);
				word.setAttribute("height",region.height+"px");
				word.style.fontSize = wordattr.fontsize;
				word.style.color = wordattr.fontcolor;
				if(wordattr.showmode=="left")
					word.style.lineHeight = region.height+"px";
				word.style.fontFamily = wordattr.fontname;
				var text = wordattr.content;
				var te=changeBR(text).replace(/ /g, "&nbsp;");
				word.innerHTML = te;
				div.appendChild(word);
				document.body.appendChild(div);
				updateEmergency(word.id);
				
				
		
		}else if(region.xtype == "sdvideo" || region.xtype == "hdvideo" || region.xtype == "audio" ){
			var playurl=igmp_addr;
		     //下面代码屏蔽 使用全部播放组播地址内容 
			 if(region.xtype=="audio")
			  	sdArrary=region.audios;
			  else
		        sdArrary=region.videos;
			if(sdArrary&&sdArrary.length>0){
				thisSW_Stb.PrintLog("===========sdurl  = " +sdArrary[sdDataPos].src);
				positionArray.push(region.x);
				positionArray.push(region.y);
				positionArray.push(region.width);
				positionArray.push(region.height);
				
					// 注释这段代码 再 正常模式下使用（不是用来发组播）
				if(igmp_addr && igmp_addr!=null && igmp_addr!=""){
					playurl=igmp_addr;
				}else{
				 if(sdArrary[sdDataPos].src.indexOf('udp:')!=-1 || sdArrary[sdDataPos].src.indexOf('igmp:')!=-1 || sdArrary[sdDataPos].src.indexOf('http:')!=-1){
						playurl=sdArrary[sdDataPos].src;
				 } else 
				 	playurl=video_server+sdArrary[sdDataPos].src;
				}
				if(positionArray.length>0){
					thisSW_MediaPlayer.setPig(positionArray[0],(positionArray[1]+10),positionArray[2],positionArray[3])
					//iPanel.ioctlWrite("media_cmd_video_pig","0,0,162, 58, 540, 452");
				}
			}
				setTimeout(function(){
						thisSW_MediaPlayer.play(playurl,true);
					//iPanel.ioctlWrite("media_localplay_start", "udp://234.0.0.9:9999");
					}, 1000)
			
		}else if (region.xtype == "time"){
			var str="<p id='time' style='color:#fff;font-size:24px;font-weight:bold;text-align:center;'></p>"
			div.style.background="transparent";
			div.innerHTML=str;
		document.body.appendChild(div);
		$(window).ready(function() {
			setInterval(function(){
				var time=new Date();
				//$("#time").html(time.format("yyyy年MM月dd日")+time.format("EEE")+time.format("HH:mm:ss"));
				$("#time").html(time.format("yyyy年MM月dd日")+"<br>"+time.format("EEE")+"<br>"+time.format("HH:mm"));
			}, 1000);
		});
		}else if(region.xtype == "weather"){
			var str="<iframe id='frame' src='"+region.weather.src+"' width=100%' height='100%' frameborder='no' border='0' scrolling='no'></iframe>"
			div.innerHTML=str;
			document.body.appendChild(div);
		}/*else if(region.xtype == "station"){
			var wordattr=region.word;
			var content=wordattr.content;
			var a=content.lastIndexOf("-");
			var id=content.substring(a+1,content.length);
			id="station_"+id;
			
			var img = document.createElement("img");
				img.id = id;
				img.style.position = "absolute";
				img.style.width = "100%";
				img.style.height = "100%";
				img.style.border = 0;
				img.src="image/1.jpg";
				div.appendChild(img);
//			var str="<img src= '' id="+id+" style='width:100%;height:100%;'></img>";
//			div.appendChild(img);
			document.body.appendChild(div);
		}*/else if(region.xtype == "imgstation"){
			var imgs = region.imgs;
			var showname=region.showname;
			var a=showname.lastIndexOf("-");
			var id=showname.substring(a+1,showname.length);
			id="station_"+id;
			var img = document.createElement("img");
			img.id = id;
			img.style.position = "absolute";
			img.style.width = "100%";
			img.style.height = "100%";
			img.style.border = 0;
//			img.src = imgs[0].src;
			div.appendChild(img);
			document.body.appendChild(div);
		}/*else if(region.xtype == "station"){
			var wordattr=region.word;
			var content=wordattr.content;
			var a=content.lastIndexOf("-");
			var id=content.substring(a+1,content.length);
			id="station_"+id;
			var str="<div id="+id+" style='width:100%;height:auto;color:"+wordattr.fontcolor+";text-align:center;font-size: "+wordattr.fontsize+"px;'></div>";
			div.innerHTML=str;
			document.body.appendChild(div);
		}*/else if(region.xtype == "datetime"){
			var wordattr=region.word;
			var dateformat=wordattr.content;
			var time=new Date();
			var str="<div style='width:100%;height:auto;color:"+wordattr.fontcolor+";text-align:center;font-size: "+wordattr.fontsize+"px;vertical-align:middle;'>"+time.format(dateformat)+"</div>";
			div.innerHTML=str;
			document.body.appendChild(div);
			$(window).ready(function() {
			new setDatetime(dateformat,region.id);
			
		});
		}else if(region.xtype== "alarminfo"){
			
			div.style.width = "100%";
			div.style.height = "100%";
			div.style.top= 0;
			div.style.left= 0;
			div.style.zIndex = 500;
			div.style.visibility= "hidden";
			div.id= "alarminfo";
			div.style.wordBreak= "break-all";
			
			document.body.appendChild(div);
			
			var width= div.offsetWidth;
			var height= div.offsetHeight;
			div.style.width= (width-400)+"px";
			div.style.height= (height-200)+"px";
			div.style.paddingTop= "100px";
			div.style.paddingBottom= "100px";
			div.style.paddingLeft= "200px";
			div.style.paddingRight= "200px";
			
			updateAlarminfo(region);
		}
	}
	
	function updateAlarminfo(region){
		var $alarminfo= $("#alarminfo");
		var wordattr=region.alarminfo;
		var oldText= ""; //上一次的告警信息
		
		
		setInterval(interval, 5*1000);
		
		function interval(){
			var ip= iPanel.ioctlRead("current_server_addr");
			//iPanel.ioctlWrite("log_print", "======1111111=="+ip+"-----");
			if(!ip){ //获取不到ip则告警层隐藏
				hideAlarm();
				return;
			}
			
			var jsonData;
			var url= "http://"+ip+"/.emergency_info?time="+new Date().getTime();
			//iPanel.ioctlWrite("log_print", "======2222222=="+url);
			var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
			ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
			ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
				if (ajax.readyState == 4) {
					
					if(ajax.status== 200){// 数据返回成功
						jsonData = ajax.responseText;
						if(jsonData){
							jsonData= jsonData.replace(new RegExp("\n","gm"),"<br/>");
							showAlarm(jsonData);
						}else{
							hideAlarm();
						}
					}else{
						hideAlarm();
					}
				}
			}
			ajax.send(null);
			
			
			
			function showAlarm(text){
				
				if(oldText== text){
					return;
				}
				oldText= text;
				var $tmpDiv= $("<div></div>"); //创建临时div，测算 告警文字 高度是否 超出父div，如果超出则只用 marquee滚动，如果不超出使用span显示全部
				$tmpDiv.css({"width": "100%", "height": "100%", "visibility": "hidden"})
				$alarminfo.append($tmpDiv);
				var $span= $("<span></span>");
				$span.css({"fontSize": wordattr.fontsize, "fontFamily": wordattr.fontname, "display": "inline-block"});
				$span.html(text);
				
				
				$tmpDiv.append($span);
				var height= $tmpDiv[0].offsetHeight- $span[0].scrollHeight;
				var width= $tmpDiv[0].offsetWidth- $span[0].scrollWidth;
				$tmpDiv.remove(); //计算完成后  删除临时元素
				$alarminfo.html("");
				if(height< 0){
					
					var $word = document.createElement("marquee");
					$word.setAttribute("id","content_" + region.id);
					$word.setAttribute("behavior","scroll");
					$word.setAttribute("scrollamount",wordattr.speed);
					$word.setAttribute("direction",wordattr.showmode);
					$word.setAttribute("height", "100%");
					$word.setAttribute("width", "100%");
					$word.style.fontSize = wordattr.fontsize;
					$word.style.color = wordattr.fontcolor;
					if(wordattr.showmode=="left")
						$word.style.lineHeight = region.height+"px";
					$word.style.fontFamily = wordattr.fontname;
					$word.innerHTML= text;
					$alarminfo[0].appendChild($word);
				}else{
					
					var $span= document.createElement("span");
					$span.id= "content_" + region.id;
					$span.style.fontSize= wordattr.fontsize;;
					$span.style.fontFamily = wordattr.fontname;
					$span.style.color = wordattr.fontcolor;
					$span.style.display= "inline-block";
					$span.style.marginTop= parseInt(height/2)+"px";
					$span.style.marginLeft= parseInt(width/2)+"px";
					$span.innerHTML= text;
					$alarminfo[0].appendChild($span);
					
				}
				
				thisSW_MediaPlayer.setMute(true);
				$alarminfo.css("visibility", "visible");
			}
			function hideAlarm(){
				if($alarminfo.css("visibility")!= "hidden"){
					thisSW_MediaPlayer.setMute(false);
					$alarminfo.css("visibility", "hidden");
					oldText= "";
				}
			}
			
			
		}
		
		
		
		
		
		
	}
	
	
	//当有紧急信息时，滚动文字 呈现紧急信息内容，  没有紧急信息时，呈现滚动文字原有内容
	function updateEmergency(domId){ 
		var $dom= $("#"+domId);
		var word= $dom.html();
		var oldText= ""; //上一次的告警信息
		
		setInterval(interval, 5*1000);
		
		function interval(){
			var ip= iPanel.ioctlRead("current_server_addr");
			//iPanel.ioctlWrite("log_print", "======1111111=="+ip+"-----");
			if(!ip){ //获取不到ip则显示原有滚动信息
				hideEmergency();
				return;
			}
			
			var jsonData;
			var url= "http://"+ip+"/.emergency_info_inferior?time="+new Date().getTime();
			//iPanel.ioctlWrite("log_print", "======2222222=="+url);
			var ajax = ajax_xmlhttp(); // 将xmlhttprequest对象赋值给一个变量．
			ajax.open("get", url, true);// 设置请求方式，请求文件，异步请求
			ajax.onreadystatechange = function() {// 你也可以这里指定一个已经写好的函数名称
				if (ajax.readyState == 4) {
					
					if(ajax.status== 200){// 数据返回成功
						jsonData = ajax.responseText;
						if(jsonData){
							jsonData= changeBR(jsonData).replace(/ /g, "&nbsp;");
							showEmergency(jsonData);
						}else{
							hideEmergency();
						}
					}else{
						hideEmergency();
					}
				}
			}
			ajax.send(null);
			
			
			
			function showEmergency(text){
				
				if(oldText== text){
					return;
				}
				oldText= text;
				$dom.html(text);
			}
			function hideEmergency(){
				$dom.html(word);
				oldText= "";
				
			}
			
			
		}
		
		
		
		
		
		
	}
	
	
	
		/**
	 * 设置时间
	 */
	function setDatetime(dateformat,id){
		setInterval(function(){
			intvatime(dateformat,id);
		}, 500);
	function intvatime(dateformat,id){		
		var time=new Date();
		var datetime = iPanel.ioctlRead("obtain_current_time");//2014-01-02 10:10:10
		if(!datetime){
		//alert(time.format(dateformat));
			$("#"+id).find("div").html(time.format(dateformat));
		}else{
			var datetimes=datetime.split(' ');
			var date=datetimes[0];//日期
			var xiaoshi=datetimes[1].split(":")[0];//小时
			var fenzhong=datetimes[1].split(":")[1];//分钟
			var miao=datetimes[1].split(":")[2];//分钟
			var arys1=date.split('-');     
			var ssdate=new Date(arys1[0],parseInt(arys1[1]-1),arys1[2],xiaoshi,fenzhong,miao);   
			
			$("#"+id).find("div").html(ssdate.format(dateformat));
			//$("#datetime").html(date+"<br>"+xingqi+"<br>"+time);
		}
	
	};
	
		
		}
	
	function imgInterval(region){
//		alert(region.id+" cccccccccc");
		var b=0;
		setInterval(function(){
				//alert(region.id+" cccccccccc");
				if(b>=region.imgs.length){
				//alert(11);
					b=0;
				}

				document.getElementById("content_" + region.id).src=region.imgs[b].src;

					b++;
				
				}, region.imgs[b].timer*1000);	
	
	}
}
		// 转换回车换行
	function changeBR(str)  {
		str=decode(str);
		if (typeof str == "string") {
			str = str.replace(/\r/g, "");
			str = str.replace(/\n/g, "<br>");
			return str;
		} else {
			return "";
		}
	}
	function decode(str) {
		if (typeof str == "string") {
			str = str.replace(/&/g, "&amp;");
			str = str.replace(/</g, "&lt;");
			str = str.replace(/>/g, "&gt;");
			return str;
		} else {
			return "";
		}
	}




if( typeof iPanel == "undefined") {
	iPanel = {
		ioctlRead : function(str) {
			return "";
		},
		ioctlWrite : function() {
		},
		mainFrame : window,
		setGlobalVar : function() {
		},
		getGlobalVar : function() {
			return "";
		}
	};
}
if( typeof Utility == "undefined") {
	Utility = {
		get : function(str) {
			return "";
		},
		set : function() {
		},
		setGlobalVar : function() {
		},
		getGlobalVar : function() {
			return "";
		}
	};
}
var thisSW_MediaPlayer = (function() {
	var mp = null;
	if( typeof MediaPlayer == "object" || typeof MediaPlayer == "function") {
	mp = new MediaPlayer();
		
	}else {
		mp =  MediaPlayerHidden.create();
	}
//		mp = new MediaPlayer();
		mp.bindNativePlayerInstance(0);
		mp.setSingleOrPlaylistMode(0);
		mp.setVideoDisplayMode(0);
		mp.setAllowTrickmodeFlag(0);
		mp.setVideoDisplayMode(0);
		mp.setNativeUIFlag(0);
		return {
			fspeed : 2.0,
			rspeed : -2.0,
			play : function(playurl, nofull) {// 播放
				var url = '[{mediaUrl:"' + playurl + '",mediaCode: "code1",mediaType:2,audioType:1,videoType:1,streamType:1,drmType:1,fingerPrint:0,copyProtection:1,allowTrickmode:1,startTime:0,endTime:0,entryID:"entry1"}]';
				mp.setSingleMedia(url);
				if( typeof nofull == "undefined" || nofull == "false")
					mp.setVideoDisplayMode(1);
				mp.playFromStart();
				this.fspeed = 2.0;
				this.rspeed = -2.0;
			},
			stop : function() {// 停止
				mp.stop();
				this.fspeed = 2.0;
				this.rspeed = -2.0;
			},
			pause : function() {// 暂停
				mp.pause();
				this.fspeed = 2.0;
				this.rspeed = -2.0;
			},
			resume : function() {// 恢复正常播放
				mp.resume();
				this.fspeed = 2.0;
				this.rspeed = -2.0;
			},
			fastForward : function() {// 快进
				mp.fastForward(this.fspeed);
				this.rspeed = -2.0;
				if(this.fspeed < 32) {
					this.fspeed = this.fspeed * 2;
				} else {
					this.fspeed = 2.0;
				}
			},
			fastRewind : function() {// 快退
				mp.fastRewind(this.rspeed);
				this.fspeed = 2.0;
				if(this.rspeed > -32.0) {
					this.rspeed = this.rspeed * 2;
				} else {
					this.rspeed = -2.0;
				}
			},
			setPig : function(x, y, w, h) {
				mp.setVideoDisplayArea(x, y, w, h);
				mp.setVideoDisplayMode(0);
				mp.refreshVideoDisplay();
			},
			setFullScreen : function() {
				mp.setVideoDisplayMode(1);
				mp.refreshVideoDisplay();
			},
			getPlaybackMode : function() {
				var jsonMode = mp.getPlaybackMode();
				jsonMode = jsonMode.replace("x", "");
				thisSW_Stb.PrintLog("jsonMode=====" + jsonMode);
				eval("jsonMode=" + jsonMode);
				var playStatus = jsonMode.PlayMode;
				if(playStatus == "Stop")
					return 0;
				if(playStatus == "Pause")
					return 1;
				if(playStatus == "NormalPlay")
					return 2;
				if(playStatus == "Trickmode")
					return 3;
				return -1;
			},
			getVolume : function() {
				var vol = mp.getVolume();
				return parseInt(vol, 10);
			},
			setVolume : function(volTemp) {
				mp.setVolume(volTemp);
			},
			getCurrentPlayTime : function() {
				return mp.getCurrentPlayTime();
			},
			getMediaDuration : function() {
				return mp.getMediaDuration();
			},
			addVolume : function() {//音量加
				var vol = mp.getVolume();
				if(vol + 3 <= 100) {
					mp.setVolume(vol + 3);
				}
			},
			decVolume : function() {//音量减
				var vol = mp.getVolume();
				if(vol - 3 >= 0) {
					mp.setVolume(vol - 3);
				}
			},
			playByTime : function(time, speed) {
				this.fspeed = 2.0;
				this.rspeed = -2.0;
				thisSW_Stb.PrintLog("playByTime time=" + time + " speed=" + speed);
				mp.playByTime(1, time, speed);
			},
			setMute: function(isMute){  //设置是否静音，  isMute=true静音   isMute=false不静音
				if(isMute){
					iPanel.ioctlWrite("audio_set_mute", "1")
				}else{
					iPanel.ioctlWrite("audio_set_mute", "0")
				}
				
			}
		}
	
})();
/*
 * 控制接口对象thisSW_Stb
 */
var thisSW_Stb = (function() {
	return {
		getAttr : function(varName)// 获取全局变量函数
		{
			if( typeof (Utility) == "object")
				return Utility.getGlobalVar(varName) || "";
			return "";
		},
		setAttr : function(varName, varValue)// 设置全局变量函数
		{
			if( typeof (Utility) == "object")
				Utility.setGlobalVar(varName, varValue);
		},
		PrintLog : function(logvalue)// 打印函数
		{
			if( typeof (logvalue) == "string")
				this.Write("log_print", "" + logvalue);
			//console.debug(logvalue);
		},
		Read : function(para, save)// 读函数
		{
			if( typeof (para) == "string") {
				var temp = "";
				if( typeof (Utility) == "object")
					temp = Utility.get(para);
				else if(para == "mac")
					temp = "00:07:63:14:46:15";
				if( typeof (save) != "undefined" && parseInt(save, 10) == 1) {
					this.setAttr(para, temp);
				}
				return temp;
			} else {
				this.PrintLog(" Para error!");
				return "";
			}
		},
		GetLanguage : function()//简体中文 zh-cn  英文 en  繁体中文 zh-tw
		{
			var lang = this.Read("language");
			if( typeof lang == "string")
				lang = lang.toLowerCase();
			this.PrintLog("==get language is :" + lang);
			return lang;
		},
		SetLanguage : function(lang)//简体中文 zh-cn  英文 en  繁体中文 zh-tw
		{
			if( typeof lang == "string" && lang != "") {
				this.PrintLog("==set language is :" + lang);
				this.Write("language", lang + "");
			} else
				this.PrintLog("set language is Error");
		},
		Write : function(cmd, para, save)// 写函数
		{
			if( typeof (cmd) == "string" && typeof (para) == "string") {
				if( typeof (save) != "undefined" && parseInt(save, 10) == 1) {
					this.setAttr(cmd, para);
				}
				if( typeof (Utility) == "object")
					return Utility.set(cmd, para + "");
				return ""
			} else {
				this.PrintLog("Not find para!");
				return "";
			}
		}
	};
})();
//语种界面管理对象
var thisStyle = (function() {
	var __LANGUAGE = {
		"zh-cn" : 0,
		"zh" : 0,
		"en" : 1,
		"zh-tw" : 2
	};
	//0 中文，1 英文，2 其他语言
	var lang = thisSW_Stb.GetLanguage();
	//语种
	var Language_Id = 0;
	if( typeof __LANGUAGE[lang] != "undefined")
		Language_Id = __LANGUAGE[lang];
	var Styles = [];
	return {
		ReloadLang : function() {
			var lang = thisSW_Stb.GetLanguage();
			//语种
			if( typeof __LANGUAGE[lang] != "undefined")
				Language_Id = __LANGUAGE[lang];
		},
		AddStyle : function(k)//object 或参数 key ，array[zh,en,ar]
		{
			var tmp = arguments;
			var obj = {
				key : "",
				con : []
			}
			if(tmp.length == 1 && typeof tmp[0] == "object") {
				var k = tmp[0];
				for(var i in k) {
					obj[i] = k[i];
				}
			} else {
				if(tmp[0])
					obj.key = tmp[0];
				if(tmp[1])
					obj.con = tmp[1];
			}
			if(obj.key != "") {
				Styles[obj.key] = obj;
				return true;
			}
			return false;
		},
		GetValue : function(key) {

			//thisSW_Stb.PrintLog("CStyle GetValue("+key+") ====start===");
			var ret = "";
			if( typeof Styles[key] == "object") {
				ret = Styles[key].con[Language_Id];
			}
			if(!ret) {
				ret = null;
				thisSW_Stb.PrintLog("Can't find " + key + " and languageid = " + Language_Id);
			}
			//thisSW_Stb.PrintLog("CStyle GetValue("+key+") ===="+ret);
			return ret;
		}
	};
})();
//===============对象扩展============
String.prototype.trim = (function() {
	var A = /^\s+|\s+$/g;
	return function() {
		return this.replace(A, "")
	}
})();
String.prototype.startWith = function(str) {
	if( typeof str == "string")
		return this.indexOf(str) == 0;
	else
		return false;
};
String.prototype.endWith = function(str) {
	if( typeof str == "string")
		return this.lastIndexOf(str) == (this.length - str.length);
	else
		return false;
};
Date.prototype.format = function(A) {
	if(!A)
		A = "yyyy/MM/dd HH:mm:ss.SSS";
	var year = this.getFullYear();
	var month = this.getMonth();
	var sMonth = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"][month];
	var sWeek = ["星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"];
	var date = this.getDate();
	var day = this.getDay();
	var hr = this.getHours();
	var min = this.getMinutes();
	var sec = this.getSeconds();
	var daysInYear = Math.ceil((this - new Date(year, 0, 0)) / 86400000);
	var weekInYear = Math.ceil((daysInYear + new Date(year, 0, 1).getDay()) / 7);
	var weekInMonth = Math.ceil((date + new Date(year, month, 1).getDay()) / 7);
	return A.replace(/yyyy/g, year).replace(/yy/g, year.toString().substr(2)).replace(/dd/g, (date < 10 ? "0" : "") + date).replace(/HH/g, (hr < 10 ? "0" : "") + hr).replace(/KK/g, (hr % 12 < 10 ? "0" : "") + hr % 12).replace(/kk/g, (hr > 0 && hr < 10 ? "0" : "") + (((hr + 23) % 24) + 1)).replace(/hh/g, (hr > 0 && hr < 10 || hr > 12 && hr < 22 ? "0" : "") + (((hr + 11) % 12) + 1)).replace(/mm/g, (min < 10 ? "0" : "") + min).replace(/ss/g, (sec < 10 ? "0" : "") + sec).replace(/SSS/g, this % 1000).replace(/a/g, (hr < 12 ? "AM" : "PM")).replace(/W/g, weekInMonth).replace(/F/g, Math.ceil(date / 7)).replace(/EEE/g, sWeek[day].substring(0, 3)).replace(/E/g, sWeek[day]).replace(/D/g, daysInYear).replace(/w/g, weekInYear).replace(/MMMM/g, sMonth).replace(/MMM/g, sMonth.substring(0, 3)).replace(/MM/g, (month < 9 ? "0" : "") + (month + 1));
};
Array.prototype.remove = function(A, isIdx) {
	if(this.length <= 0)
		return;
	if( typeof A == "number" && isIdx === true) {
		if(A >= 0 && A < this.length) {
			this.splice(A, 1);
		}
	} else {
		for(var i = 0; i < this.length; i++) {
			if(this[i] == A) {
				this.splice(i, 1);
				break;
			}
		}
	}
}
function objectToJson(obj){
	 var A = obj;
  var isArray = function(v){
      return v && typeof v.length == 'number' && typeof v.splice == 'function';
  }
  var isDate = function(v){
       return v && typeof v.getFullYear == 'function';
  }
  var pad = function(n) {
      return n < 10 ? "0" + n : n
  };
  var W = "";
  if (typeof A == "object") {
      if (isArray(A)) {
          for (var i = 0; i < A.length; i++) {
              if (typeof A[i] == "object")
                  W += (W == "" ? "" : ",") + objectToJson(A[i]);
              else if (typeof A[i] == "string")
                  W += (W == "" ? "" : ",") + "\"" + A[i].replace("\"", "\\\"") + "\"";
              else if (typeof A[i] == "number" || typeof A[i] == "boolean")
                  W += (W == "" ? "" : ",") + A[i] + "";
          }
          W = "[" + W + "]";
      } else if (isDate(A)) {
          W += "\"" + A.getFullYear() + "-" + pad(A.getMonth() + 1) + "-" + pad(A.getDate()) + "T" + pad(A.getHours()) + ":" + pad(o.getMinutes()) + ":" + pad(o.getSeconds()) + "\""
      } else {
          for (var p in A) {
              if (typeof A[p] == "object")
                  W += (W == "" ? "" : ",") +"\""+ p + "\":" + objectToJson(A[p]);
              else if (typeof A[p] == "string")
                  W += (W == "" ? "" : ",") +"\""+ p + "\":\"" + A[p].replace("\"", "\\\"") + "\"";
              else if (typeof A[p] == "number" || typeof A[p] == "boolean")
                  W += (W == "" ? "" : ",") + "\""+ p + "\":" + A[p] + "";
          }
          W = "{" + W + "}";
      }
  }
  return W;
}
//===============公共函数===============
//格式化文件大小
function formatSize(size, unit) {
	var ret = "";
	var size = parseInt(size, 10);
	var units = ["B", "K", "M", "G", "T"];
	var idx = 0;
	for(var i = 0; i < units.length; i++) {
		if(unit.toUpperCase() == units[i]) {
			idx = i;
			break;
		}
	}
	if(idx == (units.length - 1))
		return size + units[units.length - 1];
	else {
		var tmp = size;
		while(tmp > 1024) {
			tmp = parseFloat(tmp / 1024);
			idx++;
			if(idx >= (units.length - 1))
				break;
		}
		ret = formatFloat(tmp, 1) + units[idx];
	}
	return ret;
}

/**
 * 截取浮点数给定位小数
 * 第一个参数为浮点数，第二个参数为小数位数(默认1)
 * @return {}
 */
function formatFloat() {
	var A = arguments[0];
	var B = arguments[1] || 1;
	var K = Math.pow(10, B);
	var tmp = (parseFloat(A) * K + 0.5) / K;
	var ret = "" + tmp;
	ret = ret.substring(0, ret.indexOf(".") + 1 + B);
	return ret;
};

/**
 *从字符串中获取指定参数值
 * @param {} url 可选，默认为但前网页地址
 * @param {} paraKey
 * @return {String}
 */
function getPara() {
	var A = arguments;
	if(A.length <= 0)
		return "";
	var url = "";
	var paraKey = "";
	if(A.length > 1) {
		url = A[0];
		paraKey = A[1];
	} else {
		url = window.location.href;
		paraKey = A[0];
	}
	var ret = "";
	if(( typeof (url) == "string") && ( typeof (paraKey) == "string")) {
		var iPos = url.indexOf("?");
		if(iPos < 0 || iPos == url.length - 1)
			return "";
		url = url.substring(iPos + 1);
		var paras = url.split("&");
		for(var i = 0; i < paras.length; i++) {
			if(paras[i].indexOf(paraKey + "=") == 0) {
				ret = paras[i].substring(paraKey.length + 1);
				break;
			}
		}
	}
	return decodeURI(ret);
}

//除去数组中元素函数
function removeElement(index, array) {
	array.splice(index, 1);
	return array;
}

//得到字符串的真实长度（双字节换算为两个单字节）
function getStrActualLen(sChars) {

	var realLength = 0, len = sChars.length, charCode = -1;
	for(var i = 0; i < len; i++) {
		charCode = sChars.charCodeAt(i);
		if(charCode >= 0 && charCode <= 128)
			realLength += 1;
		else
			realLength += 2;
	}//62910817
	return realLength;
}

// 截取固定长度子字符串
function getInterceptedStr(sSource, iLen) {
	if(getStrActualLen(sSource) <= iLen) {
		return sSource;
	}
	var ELIDED = "...";
	var str = "";
	var l = 0;
	var schar;
	for(var i = 0; schar = sSource.charAt(i); i++) {
		if( typeof (schar) == "undefined")
			break;
		str += schar;
		l += (schar.charCodeAt(0) > 0xff ? 2 : 1);
		if(l >= iLen - ELIDED.length) {
			break;
		}
	}
	str += ELIDED;
	return str;
}

//设置样式属性值
function setStyleValue(obj, attr, val) {
	var tmp = obj.style;
	if(attr == "left" || attr == "top" || attr == "width" || attr == "height") {
		if((attr == "width" || attr == "height") && (obj.tagName == "IMG" || obj.tagName == "IMAGE"))
			obj[attr] = val;
		else
			tmp[attr] = val + "px";
	} else {
		tmp[attr] = val;
	}
}

//设置样式属性值
function getStyleValue(obj, attr) {
	var tmp = obj.style;
	if(attr == "left" || attr == "top" || attr == "width" || attr == "height") {
		if(false && (attr == "width" || attr == "height") && (obj.tagName == "IMG" || obj.tagName == "IMAGE"))
			return parseInt(obj[attr], 10);
		else
			return parseInt(tmp[attr], 10);
	} else {
		return tmp[attr];
	}
}
