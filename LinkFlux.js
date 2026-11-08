// "How can I help you, Sir?

document.getElementById("Send_Button").addEventListener("click", async function()
{
    const YT_URL = document.getElementById("videoURL").value;

    //Display/Hidden feature 

    const ErrorMsg = document.getElementById("error_msg");
    ErrorMsg.classList.add("hidden");

    const Container = document.getElementById("Container");
    const loading_status = document.getElementById("loading_status");
    
    //Display & Hidden phase 1: Hide the Container and loading status

    Container.classList.add("hidden");            
    loading_status.classList.remove("hidden");

    try 
    {
        const response = await fetch("http://localhost:18080/getVideo", 
            {
                method: "POST",
                body: YT_URL
            });

        const data = await response.json();

        document.getElementById("title").textContent = data.title;
        document.getElementById("thumbnail").src = data.thumbnail;

    //Display & Hidden phase 2: Hide the loading status, display the Container

        Container.classList.remove("hidden");            
        loading_status.classList.add("hidden");

    }
    catch(error)
    {
    //Display & Hidden unexpected phase : Hide both loading status and Container

        loading_status.classList.add("hidden");

        const ErrorMsg = document.getElementById("error_msg");
        ErrorMsg.classList.remove("hidden");
        
        console.log("Error : ", error);
    }
});

document.getElementById("Download_Button").addEventListener("click", async function()
    {
        const YT_URL = document.getElementById("videoURL").value;
        const Order = document.getElementById("Menu").value;

        const response = await fetch("http://localhost:18080/download?quality=" + Order, 
        {
            method: "POST",
            body: YT_URL
        });

        const fileName = await response.text();
        console.log("fileName:", fileName);
        const waiter = document.createElement("a"); // a = anchor, used to have download link
        waiter.href = "http://localhost:18080/file/" + fileName;
        waiter.download = fileName;
        document.body.appendChild(waiter);
        waiter.click();
        waiter.remove();
    }); 