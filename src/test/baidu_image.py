#!/usr/bin/python
# -*- coding:utf-8 -*-

import MySQLdb
import json
import urllib2
import sys
reload(sys)
sys.setdefaultencoding( "utf-8" )

if __name__ == '__main__':
	db = MySQLdb.connect('localhost', 'root', 'toor', 'Dbaidu_image')
	cursor = db.cursor()
	url_ret = urllib2.urlopen(
		'http://image.baidu.com/channel/listjson?'
		'pn=1&rn=200&tag1=%E6%98%8E%E6%98%9F&tag2=%E5%85%A8%E9%83%A8')
	dict_ret = json.load(url_ret)
	print dict_ret['tag1'].encode('utf-8')
	print dict_ret['tag2'].encode('utf-8')
	print dict_ret['totalNum']
	print dict_ret['start_index']
	print dict_ret['return_number']
	
	dict_ret_data = dict_ret['data']
	for i, item in enumerate(dict_ret_data):
		if not item.has_key('id'):
			continue
		print i
		 sql = 	""" 
		 INSERT INTO `Tdata` (
						 `Fid`, 
						 `FsetId`, 
						 `Fpn`, 
						 `Fabs`, 
						 `Fdesc`, 
						 `Ftags`, 
						 `Ftag`, 
						 `Fdate`, 
						 `Flike_num`, 
						 `Fis_single`, 

						 `Ffashion_id`, 
						 `Fdress_id`, 
						 `Ffushi_obj_num`, 
						 `Ffushi_obj_array`, 
						 `Fdress_buy_link`, 
						 `Fdress_price`, 
						 `Fdress_tag`, 
						 `Fdress_num`, 
						 `Fdress_discount`, 
						 `Fdress_other`, 

						 `Fdress_extend_type`, 
						 `Fdress_extend_name`, 
						 `Fchildren_vote`, 
						 `Fdislike_num`, 
						 `Fimage_url`, 
						 `Fimage_width`, 
						 `Fimage_height`, 
						 `Fdownload_url`, 
						 `Fthumbnail_url`, 
						 `Fthumbnail_width`, 

						 `Fthumbnail_height`, 
						 `Fthumb_large_width`, 
						 `Fthumb_large_height`, 
						 `Fthumb_large_url`, 
						 `Fsite_name`, 
						 `Fsite_logo`, 
						 `Fsite_url`, 
						 `Ffrom_url`, 
						 `Fobj_url`, 
						 `Fshare_url`, 

						 `Fdownload_num`, 
						 `Fcollect_num`, 
						 `Fstart_index`, 
						 `Freturn_number`, 
						 `Falbum_di`, 
						 `Fcan_album_id`, 
						 `Falbum_obj_num`, 
						 `Fuser_id`, 
						 `Fapp_id`, 
						 `Fcolum`, 

						 `Fphoto_id`, 
						 `Fis_album`, 
						 `Fis_vip`, 
						 `Ffrom_name`, 
						 `Fhostname`, 
						 `Fparent_tag`, 
						 `Fdesc_info`, 
						 `Fother_urls`, 
						 `FisAdapted` )
						 VALUES (
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 
										 '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s' )
						 """
		try:
			cursor.execute(sql%(
					  item['id'].encode('utf-8')
					, item['setId'].encode('utf-8')
					, item['pn']
					, item['abs'].encode('utf-8')
					, item['desc'].encode('utf-8')
					, ','.join(item['tags']).encode('utf-8')
					, item['tag'].encode('utf-8')
					, item['date'].encode('utf-8')
					, item['like_num'].encode('utf-8')
					, item['is_single'].encode('utf-8')
					, item['fashion_id'].encode('utf-8')
					, item['dress_id'].encode('utf-8')
					, item['fushi_obj_num'].encode('utf-8')
					, item['fushi_obj_array'].encode('utf-8')
					, item['dress_buy_link'].encode('utf-8')
					, item['dress_price'].encode('utf-8')
					, item['dress_tag'].encode('utf-8')
					, item['dress_num'].encode('utf-8')
					, item['dress_discount'].encode('utf-8')
					, item['dress_other'].encode('utf-8')
					, item['dress_extend_type'].encode('utf-8')
					, item['dress_extend_name'].encode('utf-8')
					, item['children_vote'].encode('utf-8')
					, item['dislike_num'].encode('utf-8')
					, item['image_url'].encode('utf-8')
					, item['image_width']
					, item['image_height']
					, item['download_url'].encode('utf-8')
					, item['thumbnail_url'].encode('utf-8')
					, item['thumbnail_width']
					, item['thumbnail_height']
					, item['thumb_large_width']
					, item['thumb_large_height']
					, item['thumb_large_url'].encode('utf-8')
					, item['site_name'].encode('utf-8')
					, item['site_logo'].encode('utf-8')
					, item['site_url'].encode('utf-8')
					, item['from_url'].encode('utf-8')
					, item['obj_url'].encode('utf-8')
					, item['share_url'].encode('utf-8')
					, item['download_num']
					, item['collect_num']
					, item['start_index']
					, item['return_number']
					, item['album_di'].encode('utf-8')
					, item['can_album_id'].encode('utf-8')
					, item['album_obj_num'].encode('utf-8')
					, item['user_id'].encode('utf-8')
					, item['app_id'].encode('utf-8')
					, item['colum'].encode('utf-8')
					, item['photo_id'].encode('utf-8')
					, item['is_album']
					, item['is_vip']
					, item['from_name']
					, item['hostname'].encode('utf-8')
					, item['parent_tag'].encode('utf-8')
					, item['desc_info'].encode('utf-8')
					, ','.join(item['other_urls']).encode('utf-8')
					, item['isAdapted']
				)
			)
			db.commit()
		except:
			db.rollback()
	# db.close()


