<?php
require(PHONE_MODULES_PATH.'grandstream/functions.inc');
class bts extends grandstream {
	function __construct() {
		global $local_config;
		if(file_exists(PHONE_MODULES_PATH."grandstream/bts/config.ini")) {
			$wfh=fopen(PHONE_MODULES_PATH."grandstream/bts/config.ini",'rb');
			$local_config = fread($wfh, filesize(PHONE_MODULES_PATH."grandstream/bts/config.ini"));
			fclose($wfh);
			$local_config = unserialize(base64_decode($local_config));
		} else {
			$data['complier_loc'] = "/usr/src/GS_CFG_GEN/bin/encode.sh";
			
			$contents = base64_encode(serialize($data));
			$wfh=fopen(PHONE_MODULES_PATH."grandstream/bts/config.ini",'w');
			fwrite($wfh,$contents);
			fclose($wfh);
			
			$local_config = $data;
		}
	}
	
	function generate_config($id) {
		global $global_cfg, $endpoint;
		
		//Get Phone Information
		$phone_info = $endpoint->get_phone_info($id);
		
		//Grandstream likes lower case letters in its mac address
		$phone_info['mac'] = strtolower($phone_info['mac']);
		//Grandstream time offset is in minutes, our global variable is in seconds
		$global_cfg['gmtoff'] = $global_cfg['gmtoff']/60;
		
		//Determine is this is a custom gui config or from a template
		if($phone_info['custom_cfg_template'] > 0) {
			$custom_cfg_data = unserialize($phone_info['template_data']['custom_cfg_data']);
			$config_files_override = unserialize($phone_info['template_data']['config_files_override']);
		} else {
			$custom_cfg_data = unserialize($phone_info['custom_cfg_data']);
			$config_files_override = unserialize($phone_info['config_files_override']);
		}
		
		$contents = $endpoint->open_config_file("\$mac.cfg",$phone_info,$config_files_override);
		
		switch(strtoupper($global_cfg['gmthr'])) {
			case "GMT-12:00":
				$global_cfg['gmthr'] = "0";
				break;
			case "GMT-11:00":
				$global_cfg['gmthr'] = "60";
				break;
			case "GMT-10:00":
				$global_cfg['gmthr'] = "120";
				break;
			case "GMT-09:00":
				$global_cfg['gmthr'] = "180";
				break;
			case "GMT-08:00":
				$global_cfg['gmthr'] = "240";
				break;
			case "GMT-07:00":
				$global_cfg['gmthr'] = "300";
				break;
			case "GMT-06:00":
				$global_cfg['gmthr'] = "360";
				break;
			case "GMT-05:00":
				$global_cfg['gmthr'] = "420";
				break;
			case "GMT-04:30":
				$global_cfg['gmthr'] = "450";
				break;
			case "GMT-04:00":
				$global_cfg['gmthr'] = "480";
				break;
			case "GMT-03:30":
				$global_cfg['gmthr'] = "510";
				break;
			case "GMT-03:00":
				$global_cfg['gmthr'] = "540";
				break;
			case "GMT-02:00":
				$global_cfg['gmthr'] = "600";
				break;
			case "GMT-01:00":
				$global_cfg['gmthr'] = "660";
				break;
			case "GMT":
				$global_cfg['gmthr'] = "720";
				break;
			case "GMT+01:00":
				$global_cfg['gmthr'] = "780";
				break;
			case "GMT+02:00":
				$global_cfg['gmthr'] = "840";
				break;
			case "GMT+03:00":
				$global_cfg['gmthr'] = "900";
				break;
			case "GMT+03:30":
				$global_cfg['gmthr'] = "930";
				break;
			case "GMT+04:00":
				$global_cfg['gmthr'] = "960";
				break;
			case "GMT+04:30":
				$global_cfg['gmthr'] = "990";
				break;
			case "GMT+05:00":
				$global_cfg['gmthr'] = "1020";
				break;
			case "GMT+05:30":
				$global_cfg['gmthr'] = "1050";
				break;
			case "GMT+05:45":
				$global_cfg['gmthr'] = "1065";
				break;
			case "GMT+06:00":
				$global_cfg['gmthr'] = "1080";
				break;
			case "GMT+06:30":
				$global_cfg['gmthr'] = "1110";
				break;
			case "GMT+07:00":
				$global_cfg['gmthr'] = "1140";
				break;
			case "GMT+08:00":
				$global_cfg['gmthr'] = "1200";
				break;
			case "GMT+09:00":
				$global_cfg['gmthr'] = "1260";
				break;
			case "GMT+09:30":
				$global_cfg['gmthr'] = "1290";
				break;
			case "GMT+10:00":
				$global_cfg['gmthr'] = "1320";
				break;
			case "GMT+11:00":
				$global_cfg['gmthr'] = "1380";
				break;
			case "GMT+12:00":
				$global_cfg['gmthr'] = "1440";
				break;
			case "GMT+13:00":
				$global_cfg['gmthr'] = "1500";
				break;
		}
		
		//Set Static variables 
		$contents=str_replace('{$ext}', $phone_info['ext'],$contents);
		$contents=str_replace('{$pass}',$phone_info['secret'], $contents);
		$contents=str_replace('{$srvip}', $global_cfg['srvip'], $contents);
		$contents=str_replace('{$gmtoff}',$global_cfg['gmthr'], $contents);
		$contents=str_replace('{$displayname}',$phone_info['description'], $contents);
		
		mkdir($global_cfg['config_location']."cfg", 0777);
		$endpoint->write_config_file("cfg/" . $phone_info['mac'] . ".txt",$phone_info,$contents,$custom_cfg_data);
		
		$outfile=$global_cfg['config_location']. "cfg" . $phone_info['mac'];
		
		if($local_config['dnt_use_gs_complier'] != ""){
			//Grandstream PHP compiler 
			$complied_cfg = $this->gs_config_out($phone_info['mac'],$this->parse_gs_config($global_cfg['config_location']."cfg/" . $phone_info['mac'] . ".txt"));
		
			//Write out the complied data from grandstream
			$wfh=fopen($outfile,'w');
			fwrite($wfh,$complied_cfg);
			fclose($wfh);
		} else {
			exec($local_config['complier_loc']." " . $phone_info['mac'] . " " . " /tftpboot/cfg/" . $phone_info['mac'].".txt " . $outfile);
		}	
		
		if(!$global_cfg['debug']) {
			unlink("cfg/" . $phone_info['mac'] . ".txt");
		}
		
		//Write out the complied data from grandstream
		$outfile=$global_cfg['config_location']. "cfg" . $phone_info['mac'];
		$wfh=fopen($outfile,w);
		fwrite($wfh,$complied_cfg);
		fclose($wfh);
		
		//Reboot the phone
		$this->reboot($id);
		
	}
	
	function delete_config($id) {
		global $global_cfg;
		
		$sql = 'SELECT mac FROM endpointman_mac_list WHERE id = '.$id;
		$result=mysql_query($sql);
		$row=mysql_fetch_array($result);
		#Grandstream likes lower case letters in its mac address
		$mac = strtolower($row['mac']);
		$outfile=$global_cfg['config_location']."cfg/" . $mac . ".txt";
		unlink($outfile);
	}
	
	function display_options(){
		global $global_cfg, $local_config;
		if($local_config['dnt_use_gs_complier'] != "") {
			$selected = "checked";
		} else {
			$selected = "";
		}
				
		$display = '<center><label>Don\'t Use The Grandstream Complier (Faster Configuration File Builds, No Encryption): <input name="dnt_use_gs_complier" type="checkbox" '.$selected.'/></label><br />
		<label>Grandstream Complier Location: <input name="complier_loc" type="textbox" id="aastra_scripts" size="50" value="'.$local_config['complier_loc'].'"/></label><br />
		<input name="bts_submit" type="submit" id="aastra_scripts" value="Save" /></center>';
		return($display);
	}
	
	function process_options(){
		global $global_cfg, $local_config;
		
		if(isset($_REQUEST['bts_submit'])) {

			$data['dnt_use_gs_complier'] = $_REQUEST['dnt_use_gs_complier'];
			$data['complier_loc'] = $_REQUEST['complier_loc'];
			
			$contents = base64_encode(serialize($data));
			$wfh=fopen(PHONE_MODULES_PATH."grandstream/bts/config.ini",'w');
			fwrite($wfh,$contents);
			fclose($wfh);
			
			$display = "<center>Saved!</center>";
		}
		return($display);
	}
}
?>