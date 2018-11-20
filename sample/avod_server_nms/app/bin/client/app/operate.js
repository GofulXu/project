(function(){
    var operate = {
        getparam : function(name,callback){
			//console.log('name:' + name);
			var execFile = require('child_process').execFile, child;
			var command = './swoperate';	
			var param_name = [];
			param_name[0] = 'GET_PARAM';
			param_name[1] = name;

			child = execFile(command, param_name, function(error, stdout, stderr) {
				//console.log('STBOUT:' + stdout);
				callback(stdout);
			});         
        },

		setparam : function(value, callback){
			var execFile = require('child_process').execFile, child;
			var command = './swoperate';	
			var param_name = [];
			param_name[0] = "SET_PARAM";
			param_name[1] = value;

			child = execFile(command, param_name, function(error, stdout, stderr) {
				console.log('STBOUT:' + stdout);
				callback(stdout);
			}); 
		},
		
		capture_screen : function(callback){
			var execFile = require('child_process').execFile, child;
			var command = './swoperate';	
			var param_name = [];
			param_name[0] = "CAPTURE_SCREEN";
			//param_name[1] = value;

			child = execFile(command, param_name, function(error, stdout, stderr) {
				console.log('STBOUT:' + stdout);
				callback(stdout);
			}); 
		},
		
		check_time : function(value,callback){
			var execFile = require('child_process').execFile, child;
			var command = './swoperate';	
			var param_name = [];
			param_name[0] = "CHECK_TIME";
			param_name[1] = value;

			child = execFile(command, param_name, function(error, stdout, stderr) {
				console.log('STBOUT:' + stdout);
				callback(stdout);
			}); 
		},
		
		restore_factory : function(callback){
			var execFile = require('child_process').execFile, child;
			var command = './swoperate';	
			var param_name = [];
			param_name[0] = "RESTORE_FACTORY";
			//param_name[1] = value;

			child = execFile(command, param_name, function(error, stdout, stderr) {
				console.log('STBOUT:' + stdout);
				callback(stdout);
			}); 
		}
    }
    module.exports = operate;
}())

