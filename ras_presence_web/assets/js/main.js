import $ from "jquery";
import * as bootstrap from 'bootstrap';
const route="https://rfidpresencev2.onrender.com/";
const members=$("#members");

$(document).ready(async ()=>{
    getPresentMembers();
})

async function getPresentMembers()
{   
    let response=await $.get(route+"get_presence_list")
    console.log(response);
    members.text("");
    for(let i in response)
    {
        let member=response[i];
        let col=`<div class="col-4 my-2"><div class="card"><div class="card-body text-center">${member.username}</div></div> </div>`;
        members.append(col);
    }
}

setInterval(getPresentMembers, 2000);
