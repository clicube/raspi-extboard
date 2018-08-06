<template>
    <div>
        <div>{{ temperature }} &deg;C</div>
        <div>{{ humidity }} %</div>
    </div>
</template>

<script>
    export default {
        computed: {
            temperature: function(){
                return this.$store.getters["envs/temperature"] || "--"
            },
            humidity: function(){
                return this.$store.getters["envs/humidity"] || "--"
            }
        }
    }
</script>
