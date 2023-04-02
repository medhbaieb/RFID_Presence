const express = require('express')
var cors = require('cors')
const mysql = require('mysql');
const util = require('util');
const bodyParser = require('body-parser');

const app = express()
const port = 3000

app.use(cors())

// parse application/x-www-form-urlencoded
app.use(bodyParser.urlencoded({ extended: false }))

// parse application/json
app.use(bodyParser.json())

const con = mysql.createConnection({
    host: "sql11.freemysqlhosting.net",
    user: "sql11591521",
    password: "BQIrIV48a2",
    database: "sql11591521"
});
  
con.connect(function(err) {
    if (err) throw err;
    console.log("Connected!");
});

const query = util.promisify(con.query).bind(con);

app.get('/', (req, res) => {
  res.send('RAS project!')
});

app.post("/get-member",async (req,res)=>{
    let code=req.body.code;
    let response;
    let result=await query("select * from users where code = ?",[code])
    let member=result[0];
    if(member)
    {
        let data={
            'name' : member.name,
            'is_present' : member.is_present
        };
        response={
            'success' : 1,
            'message' :"Member found!",
            'data' : data
        };
    }else{
        response={
            'success' : 0,
            'message' :"Member not found!",
        };
    }
    
    res.json(response);
});

app.post("/set-presence",async (req,res)=>{
    let code=req.body.code;
    let newPresence=req.body.is_present;
    let response;
    let result=await query("select * from users where code = ?",[code])
    let member=result[0];
    if(member)
    {
        await query("update users set is_present = ? WHERE code = ?",[newPresence,code])
        response={
            'success' : 1,
            'message' :"Member updatetd!",
        };
    }else{
        response={
            'success' : 0,
            'message' :"Member not found!",
        };
    }
    
    res.json(response);
});

app.get("/get-present",async (req,res)=> {
    console.log("Get present!");
    let result=await query("select name from users where is_present = 1")
    let response={
        'success' : 1,
        'message' :`Found ${result.length} member(s)`,
        'data': result
    };

    res.json(response)
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})