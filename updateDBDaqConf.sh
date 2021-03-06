#!/bin/bash
#This utility loop into /data and create one row in a list for each *.bin file
#v2.0
#
#2016-02-23 v2.0: visible satellite field added; configuration is not saved if sat<5 for less than 30 sec.
#2016-02-09 v1.7: improvement: DAQ configuration ID (daq_id) in telescopes table updated
#2016-02-04 v1.6: improvement: last Db entry is deleted right before new entries upload
#2016-02-02 v1.5: bug fix: next available data is selected only if is not current day. Added visible satellites (config. change if less than 6)
#2016-01-29 v1.4: bug fix: next available data is selected only if .bin data are available in the existing directory
#2016-01-28 v1.3: bug fix: dbEntries file is no more created if List file is empty (TOTANFILE=0)
#2016-01-26 v1.2: tmp files are moved to csv at the end to indicate that the process is ongoing
#2016-01-12 v1.1: awk script is now used to populate db entries list
#2016-01-08 v1.0: C++ e3BinDump tool is now used in place of Python script
#2015-12-04 v0.1: New logic to select analysis starting date.
#2015-12-02 v0.0: New control added when selecting starting date: analysis starts from first day with existing raw data.
#
#Created by: F. Pilo on 2015-11-30
#Usage: bash updateDBDaqConf.sh ARG1 ARG2
#
# ARG1: "ALL" - data for all telescopes in DB are analysed
# ARG2: selected date (yyyy-mm-dd) for data analysis. If missing last DB entry for each telescope or default starting date: 2015-10-27 
#

#----------------------------------------------------------------------------------------------
# Init var
#----------------------------------------------------------------------------------------------
FIRSTFILE=0
LASTFILE=0
TOTFILE=0
TOTANFILE=0

mysqlDB="mysql -h 131.154.96.193 -u eee -peee-monitoring -D eee_rundb2"
CMD0="ls"
CMD1="find"
CMD2="sort"
#CMD3="python /opt/eee/e3pipe/dst/E3BinFile.py"
CMD3="e3BinDump/e3BinDump.exe -s"
CMD4="exe"

if ls *.tmp > /dev/null 2>&1;
then 
    echo -e "\n[WARNING] TMP files found. Probably another process is still ongoing. Exiting ... "	
    exit
fi
touch exec.tmp
rm -f *.csv

if [[ -z "$E3BINDUMPENV" ]] 
then
    export E3BINDUMP=./e3BinDump
    export LD_LIBRARY_PATH=$E3BINDUMP/:${LD_LIBRARY_PATH}
    echo $LD_LIBRARY_PATH
    export E3BINBUMPENV="set"
fi  

#----------------------------------------------------------------------------------------------
# Get telescopes list
#----------------------------------------------------------------------------------------------
# TelID can be specified as:
#  - inline argument,
#  - list in a text file,
#  - downloaded from DB
#----------------------------------------------------------------------------------------------
echo -e "\n1 - Populating telescopes list ... "	
if [[ ! $1 = "ALL" ]]
then
    if [[ ! -f $1 ]]
    then
	telID="$1"
	found=$($mysqlDB -N -B -e "select name from telescopes where name='$telID';" )
	if [[ -z $found ]] 
	    then
	    echo -e "[ERROR] Telescope ID not found in DB. Exiting ...\n"
	    exit
	else
	    echo -e "[INFO] Telescope ID found in DB, OK."
	    telList=$(echo -e "$telList\n$telID")
	fi
    else
	while read telID
	do 
	    found=$($mysqlDB -N -B -e "select name from telescopes where name='$telID';" )
	    if [[ -z $found ]] 
	    then
		echo -e "[ERROR] Telescope ID not found in DB. Exiting ...\n"
		exit
	    else
		telList=$(echo -e "$telList\n$telID")
	    fi
	done < $1
	echo -e "[INFO] All telescope IDs found in DB, OK."
    fi
else
    telList=$($mysqlDB -N -B -e "select name from telescopes;" )
    if [[ -z $telList ]]
    then
	echo -e "[ERROR] No connection with the DB. Exiting ...\n"
	exit
    else
	delTelList="PISA-01"
	telList=$(echo "$telList" | sed "s@$delTelList@@")
	echo -e "[INFO] Telescope list retrieved from DB, OK."
    fi
fi

#----------------------------------------------------------------------------------------------
# Get parameteres values from RAW data directory
#----------------------------------------------------------------------------------------------
# Get new configurations from RAW data
#----------------------------------------------------------------------------------------------
echo -e "\n2 - Selecting starting date according to DB entries ..."
BINDIRS0=$($CMD0 /data/ | $CMD2)
TODAYDATE=$(date "+%F")

if [[ ! -z $2 ]]
then
    anDate=$2
    echo -e "[INFO] Analysing day $anDate, if raw data exist."
else
    for telID in $telList
    do
	
#    found=$($mysqlDB -N -B -e "select valid_until from daq_configurations join telescopes on telescope_id = telescopes.id where telescopes.name='$telID' order by valid_until desc limit 1 ;" )
	found=$($mysqlDB -N -B -e "select max(valid_until) from daq_configurations join telescopes on telescope_id = telescopes.id  where telescopes.name='$telID' group by telescopes.name;" )
#select a.gps_latitude,a.gps_longitude,a.gps_altitude,a.gps_vis_satellites,a.mrpc12_distance,a.mrpc23_distance,a.magnorth_angle,a.geonorth_angle,a.valid_from,a.valid_until,a.telescope_id,telescopes.name from daq_configurations a join telescopes on a.telescope_id = telescopes.id where a.valid_until = ( select max(valid_until) valid_until from daq_configurations b where a.telescope_id = b.telescope_id ) order by a.telescope_id;
	if [[ -z $found ]]
	then 
	    continue
	else
	    lastDBEntryDate=${found% *}
	    
	    firstAvailDataDate=""
	    let num=0
	    until [[ -n "$firstAvailDataDate" ]] || [[ "$num" -ge 365 ]]
	    do
		let num+=1

		offset="$num"
		offset+="day"		
		dataDate=$(date -d "$lastDBEntryDate+$offset" "+%F")
		#break if current day
		if [[ "$dataDate" == "$TODAYDATE" ]]
		then
		    break
		fi

		#good next available date if directory and binaries exist
		if [[ -d /data/$telID/data/$dataDate ]]
		then
		    numOfBinFiles=$(ls /data/$telID/data/$dataDate/$telID-$dataDate-*.bin | wc -w)
		    if [ $numOfBinFiles -gt 0 ]
		    then
			firstAvailDataDate="$dataDate"
			break
		    fi
		fi
	    done
	fi
	
        #echo "Now is $curFirstAvailDataDate"
	if [[ -z "$curFirstAvailDataDate" ]] || [[ ! -z $firstAvailDataDate && "$firstAvailDataDate" < "$curFirstAvailDataDate" ]]
	then
	    curFirstAvailDataDate="$firstAvailDataDate"
	    lastAnDate="$lastDBEntryDate"
	    lastAnDateTelID="$telID"
	    #echo "Updated to $curFirstAvailDataDate"
	fi
	
    done
    anDate=$curFirstAvailDataDate
    
    if [[ -z $anDate ]] 
    then
	echo -e "[WARNING] No entry found in DB. Starting from 2015-10-27."
	anDate="2015-10-28"
    else
	echo -e "[INFO] Oldest record in DB for station $lastAnDateTelID: data are valid until $lastAnDate, raw data directory for day $anDate exists."
    fi
fi

#----------------------------------------------------------------------------------------------
# Get parameteres values from RAW data directory
#----------------------------------------------------------------------------------------------
# Get new configurations from RAW data
#----------------------------------------------------------------------------------------------
echo -e "\n3 - Looking for parameter values in RAW data ... [Dir: /data]"

for telID in $telList
do

    echo -e "\n4 - $telID - Looking for parameter values in RAW data ... [Dir: /data/$telID]"
    dbTelID=$($mysqlDB -N -B -e "select id from telescopes where telescopes.name='$telID' ;" )

    found=$($mysqlDB -N -B -e "select valid_until from daq_configurations join telescopes on telescope_id = telescopes.id where telescopes.name='$telID' order by valid_until desc limit 1 ;" )
    if [[ -z $found ]]
    then
	echo -e "[WARNING] No entry found in DB for station $telID. Starting from $anDate."
    else
	lastFoundDate=${found% *}

	if [[ "$anDate" > "$lastFoundDate" ]]
	then
	    lastDBEntry=$($mysqlDB -N -B -e "select gps_latitude,gps_longitude,gps_altitude,gps_vis_satellites,mrpc12_distance,mrpc23_distance,magnorth_angle,geonorth_angle,valid_from,valid_until,telescope_id from daq_configurations where telescope_id=$dbTelID order by valid_until desc limit 1 ;" | sed 's/\t/,/g')
	    #echo $lastDBEntry
	else
	    echo -e "[INFO] Parameters already updated up to $lastFoundDate. Skipping to next station ..."
	  continue
	fi
    fi

#anDate=$(date -d "$lastAnDate+1day" "+%F")

    stationFound="false"
    for path0 in $BINDIRS0
    do

	DTELID=$(echo "$path0" | awk -F "/data/" '{print $NF}')
	if [[ -z $DTELID ]]
	then
	    continue
	fi
	
	if [ "$DTELID" != "$telID" ]
	then
	    continue
	else
	    stationFound="true"

#	    echo -e "[INFO] Raw data for station $DTELID found."
	    stdqlistfile="./daqConfListFromData"
	    stdqlistfile+="_$telID"
	    stdqlistfile+=".csv"
	    let TOTANFILE=0

	    BINDIRS1=$($CMD0 /data/$telID/data/ | $CMD2)	    
	    for dirname in $BINDIRS1
	    do

		if ! [[ "$dirname" =~ ^[[:digit:]]{4}-[[:digit:]]{2}-[[:digit:]]{2}$ ]]
		then
		    continue
		fi

		if [[ "$dirname" == "$anDate" ]]
		then

		    BINDIRS2=$($CMD1 /data/$telID/data/$dirname | $CMD2)
		    echo -e "[INFO] Analysing raw data for date $dirname"
		    lowdatetime=$(date -u -d"$dirname 00:00:00" '+%F %T')
		    updatetime=$(date -u -d"$dirname 23:59:59" '+%F %T')

		    for filenamewpath in $BINDIRS2
		    do

                        #limit on maximum number of bin files
			if [ $TOTANFILE -gt 1000 ]
			then
			    break
			fi

			FEXT=$(echo "$filenamewpath" | awk -F "." '{print $NF}')
			if [ "$FEXT" == "bin" ]
			then
			    
			    FILEID=$(echo "$filenamewpath" | awk -F "." '{print $1}')
			    FILEID=$(echo "$FILEID" | awk -F "-" '{print $NF}')

			    YEAR=$(echo "$filenamewpath" | awk -F "/" '{print $5}')
			    YEAR=$(echo "$YEAR" | awk -F "-" '{print $1}')

			    if [ $(echo "$TOTANFILE%1000" | bc) -eq 0 ]; then echo $TOTANFILE; fi
			    filename=$(echo "$filenamewpath" | awk -F "/" '{print $NF}')
			    
                   	    #Control on TELID?
	                    #gsub eliminate blanck space from datestr
			    filedatestr=$(echo "$filename"  | awk -F "-" '{gsub(" |\t",""); print $3 "-" $4 "-" $5}')
			    fileidstr_tmp=$(echo "$filename"  | awk -F "-" '{gsub(" |\t",""); print $6}')
			    fileidstr=$(echo "$fileidstr_tmp" | awk -F "." '{print $1}')
			    
			    tmpfile="./"
			    tmpfile+=$telID
			    tmpfile+="_log.txt"
			    $CMD3 $filenamewpath >> $stdqlistfile

			    let TOTANFILE++
			fi
	   	
		    done

		break
		fi
		
	    done
		       		   
	    if [ $TOTANFILE -eq 0 ] 
	    then
		echo -e "[WARNING] No raw data found for date $anDate. Skipping to next station ..."				
		continue
	    else
		echo -e "\n5 - $telID - Creating daqConfEntries files ... "	 
		
		sed -i".bak" '/ABORTED/d' $stdqlistfile #Remove "ABORTED runs" entries from input file
		totLines=$(wc -l < $stdqlistfile)
		echo -e "[INFO] Total number of lines to be analysed: $totLines"				
		dbEntriesFile="./daqConfEntries"
		dbEntriesFile+="_$telID"
		dbEntriesFile+=".csv"
		rm -f $dbEntriesFile
		touch $dbEntriesFile
		
		if [[ ! -z $lastDBEntry ]]
		then
		    echo "[INFO] Last DB entry: $lastDBEntry"
		fi

		gawk 'BEGIN { 
                               split("'"$lastDBEntry"'",curft,",");
                               curgps_latitude = curft[1]; curgps_longitude = curft[2]; curgps_altitude = curft[3]; curgps_vis_satellites = curft[4]; 
                               curmrpc12_distance = curft[5]; curmrpc23_distance = curft[6]; 
                               curmagnorth_angle = curft[7]; curgeonorth_angle = curft[8]; 
                               curlow_datetime = curft[9]; curup_datetime = curft[10]; curdbTelID = curft[11];
#printf "%.6f,%.6f,%d,%.2f,%.2f,%.1f,%.1f,%s,%s,%s\n",curgps_latitude,curgps_longitude,curgps_altitude,cur_vis_satellites,curmrpc12_distance,curmrpc23_distance,curmagnorth_angle,curgeonorth_angle,curlow_datetime,curup_datetime,curdbTelID;
                            }

                            {

                               if(curlow_datetime=="" && curup_datetime==""){
                                 curlow_datetime="2014-01-01 00:00:00"
                               }

                               split($0,ft,",");
                               filenamewpath = ft[1]; gps_latitude = ft[2]; gps_longitude = ft[3]; gps_altitude = ft[4];  gps_vis_satellites = ft[5]; mrpc12_distance = ft[6]; mrpc23_distance = ft[7]; magnorth_angle = 0; geonorth_angle = ft[8]; date = ft[9]; time = ft[10];

                               if(date=="1995-12-31" && time=="00:00:00"){
                                 gpsSync=0;
                                 elapsed_seconds=0;
                               }
                               else{

                                 mydatetime=date " " time;
                                 gsub(/[-:]/," ",mydatetime);
                                 elapsed_seconds=mktime(mydatetime)-last_update;
#printf " %d\n",elapsed_seconds;

                                 curup_datetime=date " " time;
                                 gpsSync=1;
                               }

#++lineCounter;
#fraction=int(lineCounter *100/ "'"$totLines"'");
#if(fraction%10==0) {printf "\r\tFraction of analysed lines: %d %",fraction;}

                                newCfg=0;
                                #Check different cfg
                                if(gps_latitude!=curgps_latitude){ newCfg++; }
                                if(gps_longitude!=curgps_longitude){ newCfg++; }
                                if(gps_altitude!=curgps_altitude){ newCfg++; }
                                if(gps_vis_satellites!=curgps_vis_satellites){ newCfg++; }
                                if(mrpc12_distance!=curmrpc12_distance){ newCfg++; }
                                if(mrpc23_distance!=curmrpc23_distance){ newCfg++; }
                                if(magnorth_angle!=curmagnorth_angle){ newCfg++; }
                                if(geonorth_angle!=curgeonorth_angle){ newCfg++; }

                                if(newCfg>0){

                                  if(gpsSync==1){
                                     printf "%.7f,%.7f,%d,%d,%.2f,%.2f,%.1f,%.1f,%s,%s %s,%s\n",curgps_latitude,curgps_longitude,curgps_altitude,curgps_vis_satellites,curmrpc12_distance,curmrpc23_distance,curmagnorth_angle,curgeonorth_angle,curlow_datetime,date,time,"'"$dbTelID"'"; 
                                     curlow_datetime=date " " time; 
                                  }
                                  if(gpsSync==0){
                                    printf "%.7f,%.7f,%d,%d,%.2f,%.2f,%.1f,%.1f,%s,%s,%s\n",curgps_latitude,curgps_longitude,curgps_altitude,curgps_vis_satellites,curmrpc12_distance,curmrpc23_distance,curmagnorth_angle,curgeonorth_angle,curlow_datetime,curup_datetime,"'"$dbTelID"'";
                                    curlow_datetime=curup_datetime; 
                                  }

                                  curlow_datetime_formatted = curlow_datetime;
                                  gsub(/[-:]/," ",curlow_datetime_formatted);
                                  last_update=mktime(curlow_datetime_formatted);

                                  curgps_latitude = gps_latitude; curgps_longitude = gps_longitude; curgps_altitude = gps_altitude;  curgps_vis_satellites = gps_vis_satellites;
                                  curmrpc12_distance = mrpc12_distance; curmrpc23_distance = mrpc23_distance;
                                  curmagnorth_angle = magnorth_angle; curgeonorth_angle = geonorth_angle;

                                }
                           } 

                          END {
                             printf "%.6f,%.6f,%d,%d,%.2f,%.2f,%.1f,%.1f,%s,%s,%s\n",curgps_latitude,curgps_longitude,curgps_altitude,curgps_vis_satellites,curmrpc12_distance,curmrpc23_distance,curmagnorth_angle,curgeonorth_angle,curlow_datetime,"'"$updatetime"'","'"$dbTelID"'"; 
                          }' $stdqlistfile >> $dbEntriesFile

		gawk 'BEGIN { iline=0;
                            }

                            {
                               split($0,ft,",");
                               gps_latitude[iline] = ft[1]; gps_longitude[iline] = ft[2]; gps_altitude[iline] = ft[3];  gps_vis_satellites[iline] = ft[4]; 
                               mrpc12_distance[iline] = ft[5]; mrpc23_distance[iline] = ft[6]; magnorth_angle[iline] = ft[7]; geonorth_angle[iline] = ft[8]; low_datetime[iline] = ft[9]; up_datetime[iline] = ft[10]; telID[iline] = ft[11];

                               if( iline == 1 ){


                                 newCfg=0;
                                 #Check different cfg
                                 if(gps_latitude[1]!=gps_latitude[0]){ newCfg++; }
                                 if(gps_longitude[1]!=gps_longitude[0]){ newCfg++; }
                                 if(gps_altitude[1]!=gps_altitude[0]){ newCfg++; }
                                 if(gps_vis_satellites[1]<5 || gps_vis_satellites[0]<5){ newCfg++; }
                                  if(mrpc12_distance[1]!=mrpc12_distance[0]){ newCfg++; }
                                 if(mrpc23_distance[1]!=mrpc23_distance[0]){ newCfg++; }
                                 if(magnorth_angle[1]!=magnorth_angle[0]){ newCfg++; }
                                 if(geonorth_angle[1]!=geonorth_angle[0]){ newCfg++; }

                                 if(newCfg>0){

                                  curlow_datetime_formatted = low_datetime[1];
                                  gsub(/[-:]/," ",curlow_datetime_formatted);
                                  curlow_datetime_sec=mktime(curlow_datetime_formatted);

                                  curup_datetime_formatted = up_datetime[1];
                                  gsub(/[-:]/," ",curup_datetime_formatted);
                                  curup_datetime_sec=mktime(curup_datetime_formatted);

                                  if( gps_vis_satellites[1] < 4 && ( curup_datetime_sec - curlow_datetime_sec < 30 ) ){

                                     up_datetime[0] = up_datetime[1];
                                     iline=0;

                                  }
                                  else{

                                     printf "%.7f,%.7f,%d,%d,%.2f,%.2f,%.1f,%.1f,%s,%s,%s\n",gps_latitude[0],gps_longitude[0],gps_altitude[0],gps_vis_satellites[0],mrpc12_distance[0],mrpc23_distance[0],magnorth_angle[0],geonorth_angle[0],low_datetime[0],up_datetime[0],telID[0];

                                     gps_latitude[0] = gps_latitude[1]; gps_longitude[0] = gps_longitude[1]; gps_altitude[0] = gps_altitude[1]; gps_vis_satellites[0] = gps_vis_satellites[1]; 
                                     mrpc12_distance[0] = mrpc12_distance[1]; mrpc23_distance[0] = mrpc23_distance[1]; magnorth_angle[0] = magnorth_angle[1]; geonorth_angle[0] = geonorth_angle[1]; 
                                     low_datetime[0] = low_datetime[1]; up_datetime[0] = up_datetime[1]; telID[0] = telID[1];
                                     iline=0;
                                  
                                   }
}
else{
                                     up_datetime[0] = up_datetime[1];
                                     iline=0;

}

                               }

                               iline++; 

                           } 

                          END {
                                     printf "%.7f,%.7f,%d,%d,%.2f,%.2f,%.1f,%.1f,%s,%s,%s\n",gps_latitude[0],gps_longitude[0],gps_altitude[0],gps_vis_satellites[0],mrpc12_distance[0],mrpc23_distance[0],magnorth_angle[0],geonorth_angle[0],low_datetime[0],up_datetime[0],telID[0];
                          }' $dbEntriesFile >> tmp.csv
		
		mv tmp.csv $dbEntriesFile

		if [[ ! -z $lastDBEntry ]]
		then
		    $mysqlDB -N -B -e "delete quick from daq_configurations where telescope_id='$dbTelID' order by valid_until desc limit 1;"
		fi  		

		echo -e "\n6 - $telID - Updating database ... "	 

		mysqlCMD="load data local infile '"
		mysqlCMD+="$dbEntriesFile"
		mysqlCMD+="' into table daq_configurations fields terminated by ','  lines terminated by '\n' (gps_latitude,gps_longitude,gps_altitude,gps_vis_satellites,mrpc12_distance,mrpc23_distance,magnorth_angle,geonorth_angle,valid_from,valid_until,telescope_id)"
		$mysqlDB -e "$mysqlCMD"
	
		mysqlCMD="update telescopes set daq_id = (select max(id) from daq_configurations where telescope_id="
                mysqlCMD+="$dbTelID"
                mysqlCMD+=" group by telescopes.name) where id="
                mysqlCMD+="$dbTelD"

		$mysqlDB -e "$mysqlCMD"

	    fi

	fi
	
    done

    if [[ $stationFound == "false" ]] 
    then
	echo -e "[WARNING] No raw data found for station $telID. Skipping to next station ..."				
    fi

done

echo -e "[INFO] I'm done! Exiting ..."
rm -f exec.tmp