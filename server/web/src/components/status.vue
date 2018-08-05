<template>
    <div>
        <div>{{ temperature }} &deg;C</div>
        <div>{{ humidity }} %</div>
    </div>
</template>

<script>
    import { mapGetters } from 'vuex'
    export default {
        computed: mapGetters('envs', [ 'temperature', 'humidity' ])
    }
</script>
