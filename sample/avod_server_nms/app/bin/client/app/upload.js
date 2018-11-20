(function(){	

	var fs = require("fs"), request = require("request"),pathutil = require("path");
	/**
	 * upload file 
	 * @url  upload url
	 * @path file or dir
	 */
	var uploadFile = {
		upload : function(url, paths, callback){
			var uploadfiles = [];
			
			var pathArray= paths.split('|');
			for(var i= 0, n= pathArray.length; i< n; i++){
				var path= pathArray[i];
				if(fs.statSync(path).isDirectory()){
					var files = fs.readdirSync(path);
					for(var x in files){
						var cf = pathutil.join(path, files[x])			
						if(!fs.statSync(cf).isDirectory())
							uploadfiles.push(cf);
					}
					
				}else
					uploadfiles.push(path);
			}
			
			//build formData
			var formData = {  
				my_field: 'my_value', //BUG? don't remove
				attachments: []	 
			};
			for(var x in uploadfiles){
				var bean = {
					value: fs.createReadStream(uploadfiles[x]),
					options : {
						filename : uploadfiles[x],
						contentType: 'application/octet-stream'
					}
				}
				formData['custom_' + x] = bean;		
			}	
			
			request.post({url: url, formData: formData}, function optionalCallback(err, httpResponse, body) {
				if(callback){
					callback(err);
				}
				
			  /*if (err) {
				return console.error('upload failed:', err);
			  }
			  console.log('Upload successful!  Server responded with:', body);*/
			});
		}
	}
		
	module.exports = uploadFile;
	
}())

